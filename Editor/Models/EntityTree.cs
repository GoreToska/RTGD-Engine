using System.Collections.Generic;

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
}

public readonly record struct EntityNode(string Name, long Id, long ParentId);
