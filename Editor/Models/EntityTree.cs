using System.Collections.Generic;
using System.Linq;

namespace Editor.Models;

public static class EntityTree
{
    public static List<Entity> Build(IReadOnlyList<EntityNode> nodes)
    {
        var entities = new Dictionary<long, Entity>(nodes.Count);
        foreach (var node in nodes)
            entities[node.Id] = new Entity { Id = node.Id, Name = node.Name };

        var roots = new List<Entity>();
        foreach (var node in nodes)
        {
            var entity = entities[node.Id];
            if (node.ParentId != 0 && entities.TryGetValue(node.ParentId, out var parent))
                parent.Children.Add(entity);
            else
                roots.Add(entity);
        }

        return roots;
    }

    public static List<Entity> CreateSample() => Build(
    [
        new EntityNode("Environment", 1, 0),
        new EntityNode("Sun", 2, 1),
        new EntityNode("Terrain", 3, 1),
        new EntityNode("Rocks", 4, 3),
        new EntityNode("Player", 5, 0),
        new EntityNode("Camera", 6, 5),
    ]);

    public static Entity? Find(IEnumerable<Entity> roots, long id)
    {
        foreach (var node in roots)
        {
            if (node.Id == id) return node;
            var found = Find(node.Children, id);
            if (found is not null) return found;
        }
        return null;
    }

    public static bool Rename(IEnumerable<Entity> roots, long id, string newName)
    {
        var entity = Find(roots, id);
        if (entity is null) return false;
        entity.Name = newName;
        return true;
    }

    public static bool Remove(IList<Entity> roots, long id)
    {
        if (RemoveFromChildren(roots, id)) return true;
        for (var i = 0; i < roots.Count; i++)
        {
            if (roots[i].Id != id) continue;
            roots.RemoveAt(i);
            return true;
        }
        return false;
    }

    private static bool RemoveFromChildren(IEnumerable<Entity> roots, long id)
    {
        foreach (var root in roots)
        {
            for (var i = 0; i < root.Children.Count; i++)
            {
                if (root.Children[i].Id == id)
                {
                    root.Children.RemoveAt(i);
                    return true;
                }
                if (RemoveFromChildren(root.Children[i].Children, id))
                    return true;
            }
        }
        return false;
    }

    /// <summary>
    /// Removes selected nodes. If both a parent and its descendant are selected,
    /// only the parent is removed (descendant goes with the subtree).
    /// </summary>
    public static void RemoveMany(IList<Entity> roots, IEnumerable<long> ids)
    {
        var selected = ids.ToHashSet();
        if (selected.Count == 0)
            return;

        var deletionRoots = new List<long>();
        CollectDeletionRoots(roots, selected, deletionRoots, underSelectedAncestor: false);

        foreach (var id in deletionRoots)
            Remove(roots, id);
    }

    private static void CollectDeletionRoots(
        IEnumerable<Entity> nodes,
        HashSet<long> selected,
        List<long> deletionRoots,
        bool underSelectedAncestor)
    {
        foreach (var node in nodes)
        {
            var isSelected = selected.Contains(node.Id);
            if (isSelected && !underSelectedAncestor)
                deletionRoots.Add(node.Id);

            CollectDeletionRoots(
                node.Children,
                selected,
                deletionRoots,
                underSelectedAncestor || isSelected);
        }
    }
}

public readonly record struct EntityNode(string Name, long Id, long ParentId);
