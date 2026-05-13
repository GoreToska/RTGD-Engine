using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

namespace Editor.Inspector
{
    public partial class InspectorPanel : UserControl
    {
        private ulong _selectedEntityId;
        private string _selectedEntityName;

        // Flat list of all editable value boxes so we can update them on the timer
        // without rebuilding the entire UI.
        // Tuple: (compIndex, fieldIndex, subIndex — or -1 if top-level, TextBox)
        private readonly List<(int c, int f, int s, TextBox box)> _valueBoxes = new();

        // Live-update timer polls Inspector_RefreshValues and pushes new text
        // into any TextBox that is not currently focused
        private readonly DispatcherTimer _refreshTimer;
        private const double RefreshHz = 10;

        public InspectorPanel()
        {
            InitializeComponent();

            _refreshTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1.0 / RefreshHz)
            };
            _refreshTimer.Tick += OnRefreshTick;
        }

        public void SetSelectedEntity(ulong entityId, string name)
        {
            _selectedEntityId = entityId;
            _selectedEntityName = name;
            Rebuild();
        }

        private void Rebuild()
        {
            _refreshTimer.Stop();
            _valueBoxes.Clear();
            ComponentsPanel.Children.Clear();

            if (_selectedEntityId == 0)
            {
                AddPlaceholder("No entity selected");
                return;
            }

            HeaderTitle.Text = $"Inspector — {_selectedEntityName}";

            int count = NativeInterop.Inspector_GetComponentCount(_selectedEntityId);
            if (count == 0) { AddPlaceholder("No components"); return; }

            for (int c = 0; c < count; c++)
            {
                var comp = new ComponentViewModel
                {
                    Name = NativeInterop.GetComponentName(_selectedEntityId, c)
                };

                int fieldCount = NativeInterop.Inspector_GetFieldCount(c);
                for (int f = 0; f < fieldCount; f++)
                {
                    var field = new FieldViewModel
                    {
                        Name = NativeInterop.GetFieldName(c, f),
                        Type = NativeInterop.GetFieldType(c, f),
                        Value = NativeInterop.GetFieldValue(c, f)
                    };

                    int subCount = NativeInterop.Inspector_GetSubFieldCount(c, f);
                    for (int s = 0; s < subCount; s++)
                    {
                        field.Children.Add(new FieldViewModel
                        {
                            Name = NativeInterop.GetSubFieldName(c, f, s),
                            Type = NativeInterop.GetSubFieldType(c, f, s),
                            Value = NativeInterop.GetSubFieldValue(c, f, s)
                        });
                    }
                    comp.Fields.Add(field);
                }

                ComponentsPanel.Children.Add(CreateComponentBlock(comp, c));
            }

            _refreshTimer.Start();
        }

        private void OnRefreshTick(object sender, EventArgs e)
        {
            if (_selectedEntityId == 0) return;
            if (NativeInterop.Inspector_RefreshValues() == 0) return;

            foreach (var (c, f, s, box) in _valueBoxes)
            {
                if (box.IsFocused) continue;

                string newVal = s < 0
                    ? NativeInterop.GetFieldValue(c, f)
                    : NativeInterop.GetSubFieldValue(c, f, s);

                if (box.Text != newVal)
                    box.Text = newVal;
            }
        }

        private void AddPlaceholder(string text)
        {
            ComponentsPanel.Children.Add(new TextBlock
            {
                Text = text,
                Foreground = Brushes.Gray,
                FontStyle = FontStyles.Italic,
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                Margin = new Thickness(10, 20, 10, 0)
            });
        }

        private UIElement CreateComponentBlock(ComponentViewModel comp, int compIndex)
        {
            var border = new Border
            {
                Background = new SolidColorBrush(Color.FromRgb(0x25, 0x25, 0x26)),
                BorderBrush = new SolidColorBrush(Color.FromRgb(0x3E, 0x3E, 0x42)),
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(4),
                Margin = new Thickness(0, 0, 0, 4)
            };

            var expander = new Expander
            {
                IsExpanded = true,
                Foreground = Brushes.LightGray,
                Background = Brushes.Transparent,
                BorderThickness = new Thickness(0),
                Padding = new Thickness(4),
                Header = new TextBlock
                {
                    Text = comp.Name,
                    FontSize = 12,
                    FontWeight = FontWeights.SemiBold,
                    Foreground = new SolidColorBrush(Color.FromRgb(0x9C, 0xDC, 0xFE))
                }
            };

            var rows = FlattenFields(comp.Fields);

            var grid = new Grid { Margin = new Thickness(8, 2, 8, 6) };
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });

            if (rows.Count == 0)
            {
                grid.RowDefinitions.Add(new RowDefinition());
                var noFields = new TextBlock
                {
                    Text = "No reflected fields",
                    Foreground = Brushes.Gray,
                    FontStyle = FontStyles.Italic,
                    FontSize = 11
                };
                Grid.SetColumnSpan(noFields, 2);
                grid.Children.Add(noFields);
            }

            for (int i = 0; i < rows.Count; i++)
            {
                var (field, depth, fieldIdx, subIdx) = rows[i];
                grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(22) });

                if (i % 2 == 0)
                {
                    var rowBg = new Border
                    {
                        Background = new SolidColorBrush(Color.FromArgb(0x18, 0xFF, 0xFF, 0xFF))
                    };
                    Grid.SetColumnSpan(rowBg, 2);
                    Grid.SetRow(rowBg, i);
                    grid.Children.Add(rowBg);
                }

                double leftPad = 4 + depth * 14;

                if (field.IsStruct)
                {
                    var header = new TextBlock
                    {
                        Text = field.Name,
                        Foreground = new SolidColorBrush(Color.FromRgb(0xDC, 0xDC, 0xAA)),
                        FontSize = 11,
                        FontWeight = FontWeights.SemiBold,
                        VerticalAlignment = VerticalAlignment.Center,
                        Margin = new Thickness(leftPad, 0, 0, 0),
                        ToolTip = field.Type
                    };
                    Grid.SetRow(header, i);
                    Grid.SetColumnSpan(header, 2);
                    grid.Children.Add(header);
                }
                else
                {
                    var nameBlock = new TextBlock
                    {
                        Text = field.Name,
                        Foreground = new SolidColorBrush(Color.FromRgb(0xCC, 0xCC, 0xCC)),
                        FontSize = 11,
                        VerticalAlignment = VerticalAlignment.Center,
                        Margin = new Thickness(leftPad, 0, 0, 0),
                        ToolTip = field.Type
                    };

                    int ci = compIndex, fi = fieldIdx, si = subIdx;

                    var valueBox = new TextBox
                    {
                        Text = field.Value,
                        Foreground = GetTypeColor(field.Type),
                        Background = Brushes.Transparent,
                        BorderThickness = new Thickness(0),
                        FontSize = 11,
                        FontFamily = new FontFamily("Consolas"),
                        VerticalAlignment = VerticalAlignment.Center,
                        Margin = new Thickness(4, 1, 2, 1),
                        Padding = new Thickness(0),
                        CaretBrush = Brushes.White,
                        SelectionBrush = new SolidColorBrush(Color.FromArgb(0x60, 0x26, 0x7A, 0xCC))
                    };

                    // Commit on Enter or lost focus
                    valueBox.KeyDown += (_, e) =>
                    {
                        if (e.Key == Key.Enter) CommitValue(ci, fi, si, valueBox);
                    };
                    valueBox.LostFocus += (_, _) => CommitValue(ci, fi, si, valueBox);

                    valueBox.GotFocus += (_, _) => valueBox.BorderThickness = new Thickness(0, 0, 0, 1);
                    valueBox.LostFocus += (_, _) => valueBox.BorderThickness = new Thickness(0);

                    // Register for live updates
                    _valueBoxes.Add((ci, fi, si, valueBox));

                    Grid.SetRow(nameBlock, i); Grid.SetColumn(nameBlock, 0);
                    Grid.SetRow(valueBox, i); Grid.SetColumn(valueBox, 1);
                    grid.Children.Add(nameBlock);
                    grid.Children.Add(valueBox);
                }
            }

            expander.Content = grid;
            border.Child = expander;
            return border;
        }


        private static void CommitValue(int c, int f, int s, TextBox box)
        {
            string text = box.Text.Trim();

            int result = s < 0
                ? NativeInterop.Inspector_SetFieldValue(c, f, text)
                : NativeInterop.Inspector_SetSubFieldValue(c, f, s, text);

            box.Foreground = result == 1
                ? GetTypeColor(box.Tag as string ?? "")
                : Brushes.Tomato;
        }


        private static List<(FieldViewModel field, int depth, int fieldIdx, int subIdx)>
            FlattenFields(IEnumerable<FieldViewModel> fields, int depth = 0, int parentFieldIdx = 0)
        {
            var result = new List<(FieldViewModel, int, int, int)>();
            int fi = 0;
            foreach (var f in fields)
            {
                // subIdx == -1 means top-level field, use Inspector_GetFieldValue
                result.Add((f, depth, parentFieldIdx + fi, -1));
                if (f.IsStruct)
                {
                    int fi2 = parentFieldIdx + fi;
                    int si = 0;
                    foreach (var child in f.Children)
                    {
                        result.Add((child, depth + 1, fi2, si));
                        si++;
                    }
                }
                fi++;
            }
            return result;
        }

        private static Brush GetTypeColor(string type) => type switch
        {
            "float" or "double" => new SolidColorBrush(Color.FromRgb(0xB5, 0xCE, 0xA8)),
            "int32" or "int64" => new SolidColorBrush(Color.FromRgb(0xB8, 0xD7, 0xA3)),
            "uint32" or "uint64" => new SolidColorBrush(Color.FromRgb(0xB8, 0xD7, 0xA3)),
            "bool" => new SolidColorBrush(Color.FromRgb(0x56, 0x9C, 0xD6)),
            _ => Brushes.LightGray
        };
    }
}