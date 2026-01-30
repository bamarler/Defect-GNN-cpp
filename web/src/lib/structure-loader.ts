export interface Material {
  id: string;
  name: string;
  pristine: string;
  defects: string[];
}

export interface StructureManifest {
  materials: Material[];
}

export interface StructureOption {
  id: string;
  label: string;
  filename: string;
  materialId: string;
  materialName: string;
  type: 'pristine' | 'defect';
}

let cachedManifest: StructureManifest | null = null;

export async function loadManifest(): Promise<StructureManifest> {
  if (cachedManifest) {
    return cachedManifest;
  }

  const response = await fetch('/data/structures.json');
  if (!response.ok) {
    throw new Error(`Failed to load manifest: ${response.status}`);
  }

  cachedManifest = await response.json();
  return cachedManifest!;
}

export function getStructureOptions(manifest: StructureManifest): StructureOption[] {
  const options: StructureOption[] = [];

  for (const material of manifest.materials) {
    options.push({
      id: `${material.id}-pristine`,
      label: `${material.name} (pristine)`,
      filename: material.pristine,
      materialId: material.id,
      materialName: material.name,
      type: 'pristine',
    });

    material.defects.forEach((defect, idx) => {
      options.push({
        id: `${material.id}-defect-${idx + 1}`,
        label: `${material.name} (defect ${idx + 1})`,
        filename: defect,
        materialId: material.id,
        materialName: material.name,
        type: 'defect',
      });
    });
  }

  return options;
}

export async function loadStructureFile(filename: string): Promise<string> {
  const response = await fetch(`/data/structures/${filename}`);
  if (!response.ok) {
    throw new Error(`Failed to load structure: ${response.status}`);
  }
  return response.text();
}
