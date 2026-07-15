using System;
using System.Collections.Generic;
using System.Globalization;
using Avalonia.Data.Converters;

namespace Editor.Converters;

public sealed class EntityRenamingConverter : IMultiValueConverter
{
    public object? Convert(IList<object?> values, Type targetType, object? parameter, CultureInfo culture)
    {
        if (values.Count < 2 || ToLong(values[0]) is not long entityId)
            return parameter as string != "edit";

        var isEditing = ToLong(values[1]) is long renamingId && renamingId == entityId;
        var wantEdit = parameter as string == "edit";
        return wantEdit ? isEditing : !isEditing;
    }

    private static long? ToLong(object? value) => value switch
    {
        long l => l,
        int i => i,
        _ => null
    };
}
