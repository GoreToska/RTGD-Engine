using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Editor.Models;

namespace Editor.ViewModels.Panels;

public partial class HierarchyViewModel : ViewModelBase
{
    public ObservableCollection<Entity> Entities { get; } = new();

    [ObservableProperty]
    private long? renamingEntityId;

    [ObservableProperty]
    private string renameDraft = string.Empty;

    public Action<long, string>? OnRenameRequested;
    public Action<IReadOnlyList<long>>? OnDeleteRequested;

    public event Action<IReadOnlyList<long>>? SelectionSyncRequested;

    private List<long> _selectedEntityIds = [];
    private List<long> _renameSelectionIds = [];

    public IReadOnlyList<long> SelectedEntityIds => _selectedEntityIds;

    private bool CanRename() => _selectedEntityIds.Count == 1 && RenamingEntityId is null;

    private bool CanDelete() => _selectedEntityIds.Count > 0 && RenamingEntityId is null;

    public void SetSelectedIds(IReadOnlyList<long> ids)
    {
        _selectedEntityIds = ids.Distinct().ToList();
        NotifySelectionCommandsChanged();
    }

    [RelayCommand(CanExecute = nameof(CanRename))]
    private void RenameEntity()
    {
        if (_selectedEntityIds.Count != 1)
            return;

        var id = _selectedEntityIds[0];
        if (EntityTree.Find(Entities, id) is not { } entity)
            return;

        _renameSelectionIds = _selectedEntityIds.ToList();
        RenamingEntityId = entity.Id;
        RenameDraft = entity.Name;
    }

    [RelayCommand]
    private void CommitRename()
    {
        if (RenamingEntityId is not long id)
            return;

        var name = RenameDraft.Trim();
        if (string.IsNullOrEmpty(name))
        {
            CancelRename();
            return;
        }

        OnRenameRequested?.Invoke(id, name);
        ClearRenameState();
    }

    [RelayCommand]
    private void CancelRename() => ClearRenameState();

    public IReadOnlyList<long> TakeRenameSelectionRestore()
    {
        var ids = _renameSelectionIds;
        _renameSelectionIds = [];
        return ids;
    }

    public void ClearRenameSelectionRestore() => _renameSelectionIds = [];

    [RelayCommand(CanExecute = nameof(CanDelete))]
    private void DeleteEntity()
    {
        if (_selectedEntityIds.Count == 0)
            return;

        var ids = _selectedEntityIds.ToList();
        OnDeleteRequested?.Invoke(ids);
        SetSelectedIds([]);
        RequestSelectionSync([]);
    }

    public void SetEntities(IEnumerable<Entity> roots)
    {
        Entities.Clear();

        foreach (var entity in roots)
            Entities.Add(entity);

        ClearRenameState();
        _renameSelectionIds = [];
        PruneSelection();
        // Always resync: Entity instances are new after rebuild.
        RequestSelectionSync(_selectedEntityIds);
    }

    public void Clear()
    {
        Entities.Clear();
        SetSelectedIds([]);
        RequestSelectionSync([]);
        ClearRenameState();
        _renameSelectionIds = [];
    }

    partial void OnRenamingEntityIdChanged(long? value) =>
        NotifySelectionCommandsChanged();

    private void PruneSelection()
    {
        var pruned = _selectedEntityIds
            .Where(id => EntityTree.Find(Entities, id) is not null)
            .ToList();

        if (pruned.Count == _selectedEntityIds.Count)
            return;

        SetSelectedIds(pruned);
    }

    private void RequestSelectionSync(IReadOnlyList<long> ids) =>
        SelectionSyncRequested?.Invoke(ids);

    private void NotifySelectionCommandsChanged()
    {
        RenameEntityCommand.NotifyCanExecuteChanged();
        DeleteEntityCommand.NotifyCanExecuteChanged();
    }

    private void ClearRenameState()
    {
        RenamingEntityId = null;
        RenameDraft = string.Empty;
        NotifySelectionCommandsChanged();
    }
}
