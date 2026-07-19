using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Threading;
using Avalonia.VisualTree;
using Editor.Models;
using Editor.ViewModels.Panels;

namespace Editor.Views.Panels;

public partial class HierarchyPanel : UserControl
{
    private HierarchyViewModel? _viewModel;
    private bool _suppressSelectionSync;
    private bool _isCommittingRename;

    public HierarchyPanel()
    {
        InitializeComponent();
        DataContextChanged += OnDataContextChanged;
        PointerPressed += OnPointerPressed;
        HierarchyTree.SelectionChanged += OnTreeSelectionChanged;
    }

    private void OnDataContextChanged(object? sender, System.EventArgs e)
    {
        if (_viewModel is not null)
        {
            _viewModel.PropertyChanged -= OnViewModelPropertyChanged;
            _viewModel.SelectionSyncRequested -= ApplySelectionToTree;
        }

        _viewModel = DataContext as HierarchyViewModel;
        if (_viewModel is not null)
        {
            _viewModel.PropertyChanged += OnViewModelPropertyChanged;
            _viewModel.SelectionSyncRequested += ApplySelectionToTree;
        }
    }

    private void OnViewModelPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName != nameof(HierarchyViewModel.RenamingEntityId) || _viewModel is null)
            return;

        if (_viewModel.RenamingEntityId is not null)
        {
            Dispatcher.UIThread.Post(FocusRenameTextBox, DispatcherPriority.Loaded);
            return;
        }

        Dispatcher.UIThread.Post(RestoreAfterRename, DispatcherPriority.Loaded);
    }

    private void OnTreeSelectionChanged(object? sender, SelectionChangedEventArgs e)
    {
        if (_suppressSelectionSync || _viewModel is null)
            return;

        var ids = HierarchyTree.SelectedItems
            .OfType<Entity>()
            .Select(entity => entity.Id)
            .ToList();

        _viewModel.SetSelectedIds(ids);
    }

    private void RestoreAfterRename()
    {
        if (_viewModel is null || _viewModel.RenamingEntityId is not null)
            return;

        var ids = _viewModel.TakeRenameSelectionRestore();
        if (ids.Count > 0)
            ApplySelectionToTree(ids);

        FocusSelectedTreeItem();
    }

    private void FocusSelectedTreeItem()
    {
        if (_viewModel is null || _viewModel.SelectedEntityIds.Count == 0)
        {
            HierarchyTree.Focus();
            return;
        }

        var entity = _viewModel.SelectedEntityIds
            .Select(id => EntityTree.Find(_viewModel.Entities, id))
            .FirstOrDefault(candidate => candidate is not null);

        if (entity is null)
        {
            HierarchyTree.Focus();
            return;
        }

        ExpandPath(_viewModel.Entities, entity);
        if (HierarchyTree.TreeContainerFromItem(entity) is TreeViewItem item)
        {
            item.IsSelected = true;
            item.Focus();
            return;
        }

        HierarchyTree.Focus();
        Dispatcher.UIThread.Post(() =>
        {
            if (HierarchyTree.TreeContainerFromItem(entity) is TreeViewItem deferredItem)
            {
                deferredItem.IsSelected = true;
                deferredItem.Focus();
            }
        }, DispatcherPriority.Render);
    }

    private void ApplySelectionToTree(IReadOnlyList<long> ids)
    {
        if (_viewModel is null)
            return;

        var entities = ids
            .Select(id => EntityTree.Find(_viewModel.Entities, id))
            .Where(entity => entity is not null)
            .Cast<Entity>()
            .ToList();

        _suppressSelectionSync = true;
        try
        {
            HierarchyTree.UnselectAll();
            foreach (var entity in entities)
            {
                ExpandPath(_viewModel.Entities, entity);
                HierarchyTree.SelectedItems.Add(entity);
            }
        }
        finally
        {
            _suppressSelectionSync = false;
        }

        _viewModel.SetSelectedIds(entities.Select(entity => entity.Id).ToList());
    }

    private bool ExpandPath(IEnumerable<Entity> nodes, Entity target)
    {
        foreach (var node in nodes)
        {
            if (node.Id == target.Id)
                return true;

            if (!ExpandPath(node.Children, target))
                continue;

            if (HierarchyTree.TreeContainerFromItem(node) is TreeViewItem container)
                container.IsExpanded = true;

            return true;
        }

        return false;
    }

    private void FocusRenameTextBox()
    {
        foreach (var textBox in HierarchyTree.GetVisualDescendants().OfType<TextBox>())
        {
            if (!textBox.IsVisible || !textBox.Classes.Contains("rename-field"))
                continue;

            textBox.Focus();
            textBox.SelectAll();
            return;
        }
    }

    private void CommitRenameFromView()
    {
        if (DataContext is not HierarchyViewModel vm
            || vm.RenamingEntityId is null
            || _isCommittingRename)
            return;

        _isCommittingRename = true;
        try
        {
            vm.CommitRenameCommand.Execute(null);
        }
        finally
        {
            _isCommittingRename = false;
        }
    }

    private void OnRenameKeyDown(object? sender, KeyEventArgs e)
    {
        if (DataContext is not HierarchyViewModel vm)
            return;

        if (e.Key == Key.Enter)
        {
            CommitRenameFromView();
            e.Handled = true;
        }
        else if (e.Key == Key.Escape)
        {
            vm.CancelRenameCommand.Execute(null);
            e.Handled = true;
        }
    }

    private void OnRenameLostFocus(object? sender, RoutedEventArgs e) =>
        CommitRenameFromView();

    private void OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        if (DataContext is not HierarchyViewModel vm || vm.RenamingEntityId is null)
            return;

        if (e.Source is not Control source || IsInsideActiveRenameTextBox(source))
            return;

        if (source.FindAncestorOfType<TreeViewItem>() is not null)
            vm.ClearRenameSelectionRestore();

        CommitRenameFromView();
    }

    private static bool IsInsideActiveRenameTextBox(Control control)
    {
        var textBox = control.FindAncestorOfType<TextBox>();
        return textBox is { IsVisible: true } && textBox.Classes.Contains("rename-field");
    }
}
