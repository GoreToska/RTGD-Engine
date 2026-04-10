using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor.Inspector
{
    /// <summary>
    /// Interaction logic for InspectorPanel.xaml
    /// </summary>
    public partial class InspectorPanel : UserControl
    {
        private ulong _selectedEntityId;
        private string _selectedEntityName;

        public InspectorPanel()
        {
            InitializeComponent();
        }


        public void SetSelectedEntity(ulong entityId, string name)
        {
            _selectedEntityId = entityId;
            _selectedEntityName = name;
            Refresh();
        }


        public void Refresh()
        {
            ComponentsPanel.Children.Clear();

            if (_selectedEntityId == 0) { AddPlaceholder("No entity selected"); return; }

            HeaderTitle.Text = $"Inspector — {_selectedEntityName}";

            int count = NativeInterop.Inspector_GetComponentCount(_selectedEntityId);
            if (count == 0) { AddPlaceholder("No components"); return; }

            for (int c = 0; c < count; c++)
            {
                var comp = new ComponentViewModel { Name = NativeInterop.GetComponentName(c) };

                int fieldCount = NativeInterop.Inspector_GetFieldCount(c);
                for (int f = 0; f < fieldCount; f++)
                {
                    comp.Fields.Add(new FieldViewModel
                    {
                        Name = NativeInterop.GetFieldName(c, f),
                        Type = NativeInterop.GetFieldType(c, f),
                        Value = NativeInterop.GetFieldValue(c, f)
                    });
                }

                ComponentsPanel.Children.Add(CreateComponentBlock(comp));
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

        private UIElement CreateComponentBlock(ComponentViewModel comp)
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
                Padding = new Thickness(4, 4, 4, 4),
                Header = new TextBlock
                {
                    Text = comp.Name,
                    FontSize = 12,
                    FontWeight = FontWeights.SemiBold,
                    Foreground = new SolidColorBrush(Color.FromRgb(0x9C, 0xDC, 0xFE))
                }
            };

            var grid = new Grid { Margin = new Thickness(8, 2, 8, 6) };
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });

            if (comp.Fields.Count == 0)
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

            for (int i = 0; i < comp.Fields.Count; i++)
            {
                var field = comp.Fields[i];
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

                var nameBlock = new TextBlock
                {
                    Text = field.Name,
                    Foreground = new SolidColorBrush(Color.FromRgb(0xCC, 0xCC, 0xCC)),
                    FontSize = 11,
                    VerticalAlignment = VerticalAlignment.Center,
                    Margin = new Thickness(4, 0, 0, 0),
                    ToolTip = field.Type
                };

                var valueBlock = new TextBlock
                {
                    Text = field.Value,
                    Foreground = GetTypeColor(field.Type),
                    FontSize = 11,
                    FontFamily = new FontFamily("Consolas"),
                    VerticalAlignment = VerticalAlignment.Center,
                    Margin = new Thickness(4, 0, 0, 0)
                };

                Grid.SetRow(nameBlock, i); Grid.SetColumn(nameBlock, 0);
                Grid.SetRow(valueBlock, i); Grid.SetColumn(valueBlock, 1);
                grid.Children.Add(nameBlock);
                grid.Children.Add(valueBlock);
            }

            expander.Content = grid;
            border.Child = expander;
            return border;
        }

        private Brush GetTypeColor(string type) => type switch
        {
            "float" or "double" => new SolidColorBrush(Color.FromRgb(0xB5, 0xCE, 0xA8)),
            "int32" or "int64" => new SolidColorBrush(Color.FromRgb(0xB8, 0xD7, 0xA3)),
            "uint32" or "uint64" => new SolidColorBrush(Color.FromRgb(0xB8, 0xD7, 0xA3)),
            "bool" => new SolidColorBrush(Color.FromRgb(0x56, 0x9C, 0xD6)),
            _ => Brushes.LightGray
        };
    }
}
