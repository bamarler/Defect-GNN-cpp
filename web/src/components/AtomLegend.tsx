import { getColorByElementIndex } from '@/lib/atoms';

interface AtomLegendProps {
  elements: string[];
  elementCounts: number[];
}

export function AtomLegend({ elements, elementCounts }: AtomLegendProps): React.ReactElement {
  const totalAtoms = elementCounts.reduce((sum, count) => sum + count, 0);

  return (
    <div className="glass-card pointer-events-auto p-4">
      <h3 className="text-text-primary mb-3 text-sm font-semibold">Atoms ({totalAtoms})</h3>
      <div className="space-y-2">
        {elements.map((element, idx) => (
          <div key={element} className="flex items-center gap-3">
            <div
              className="h-4 w-4 rounded-full"
              style={{ backgroundColor: getColorByElementIndex(idx) }}
            />
            <span className="text-text-primary w-8 text-sm font-medium">{element}</span>
            <span className="text-text-muted text-sm">{elementCounts[idx]}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
