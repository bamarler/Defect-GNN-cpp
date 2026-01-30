'use client';

import { StructureSelector } from './StructureSelector';
import type { StructureOption, Material } from '@/lib/structure-loader';

interface ControlsPanelProps {
  rCutoff: number;
  setRCutoff: (value: number) => void;
  maxNeighbors: number;
  setMaxNeighbors: (value: number) => void;
  showGraph: boolean;
  setShowGraph: (value: boolean) => void;
  selectedId: string;
  setSelectedId: (value: string) => void;
  structureOptions: StructureOption[];
  materials: Material[];
}

export function ControlsPanel({
  rCutoff,
  setRCutoff,
  maxNeighbors,
  setMaxNeighbors,
  showGraph,
  setShowGraph,
  selectedId,
  setSelectedId,
  structureOptions,
  materials,
}: ControlsPanelProps): React.ReactElement {
  return (
    <div className="glass-card pointer-events-auto w-64 p-4">
      <h3 className="text-text-primary mb-4 text-sm font-semibold">Controls</h3>

      <div className="space-y-4">
        <div>
          <label className="text-text-muted mb-1 block text-xs">Structure</label>
          <StructureSelector
            options={structureOptions}
            materials={materials}
            selectedId={selectedId}
            onChange={setSelectedId}
            className="w-full"
          />
        </div>

        <div className="flex items-center justify-between">
          <span className="text-text-muted text-xs">Show graph</span>
          <button
            onClick={(): void => setShowGraph(!showGraph)}
            className={`h-6 w-11 rounded-full transition-colors ${
              showGraph ? 'bg-primary-mid' : 'bg-gray-600'
            }`}
          >
            <div
              className={`h-5 w-5 rounded-full bg-white transition-transform ${
                showGraph ? 'translate-x-5' : 'translate-x-0.5'
              }`}
            />
          </button>
        </div>

        {showGraph && (
          <>
            <div>
              <label className="text-text-muted mb-1 flex justify-between text-xs">
                <span>r_cutoff</span>
                <span>{rCutoff.toFixed(1)} Å</span>
              </label>
              <input
                type="range"
                min="2"
                max="10"
                step="0.5"
                value={rCutoff}
                onChange={(e): void => setRCutoff(parseFloat(e.target.value))}
                className="w-full accent-primary-mid"
              />
            </div>

            <div>
              <label className="text-text-muted mb-1 flex justify-between text-xs">
                <span>max_neighbors</span>
                <span>{maxNeighbors}</span>
              </label>
              <input
                type="range"
                min="4"
                max="24"
                step="1"
                value={maxNeighbors}
                onChange={(e): void => setMaxNeighbors(parseInt(e.target.value))}
                className="w-full accent-primary-mid"
              />
            </div>
          </>
        )}
      </div>
    </div>
  );
}
