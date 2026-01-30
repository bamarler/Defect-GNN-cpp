import type { StructureOption, Material } from '@/lib/structure-loader';

interface StructureSelectorProps {
  options: StructureOption[];
  materials: Material[];
  selectedId: string;
  onChange: (id: string) => void;
  className?: string;
}

export function StructureSelector({
  options,
  materials,
  selectedId,
  onChange,
  className = '',
}: StructureSelectorProps): React.ReactElement {
  return (
    <select
      value={selectedId}
      onChange={(e): void => onChange(e.target.value)}
      className={`bg-bg-surface border-primary-dark text-text-primary rounded border px-2 py-1 text-sm ${className}`}
    >
      {materials.map((material) => {
        const materialOptions = options.filter((o) => o.materialId === material.id);
        return (
          <optgroup key={material.id} label={material.name}>
            {materialOptions.map((opt) => (
              <option key={opt.id} value={opt.id}>
                {material.name} (
                {opt.type === 'pristine' ? 'Pristine' : `Defect ${opt.id.split('-').pop()}`})
              </option>
            ))}
          </optgroup>
        );
      })}
    </select>
  );
}
