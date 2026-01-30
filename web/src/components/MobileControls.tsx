'use client';

import { useState } from 'react';
import { StructureSelector } from './StructureSelector';
import type { StructureOption, Material } from '@/lib/structure-loader';

interface MobileControlsProps {
  rCutoff: number;
  setRCutoff: (value: number) => void;
  maxNeighbors: number;
  setMaxNeighbors: (value: number) => void;
  showEdges: boolean;
  setShowEdges: (value: boolean) => void;
  selectedId: string;
  setSelectedId: (value: string) => void;
  structureOptions: StructureOption[];
  materials: Material[];
}

export function MobileControls({
  rCutoff,
  setRCutoff,
  maxNeighbors,
  setMaxNeighbors,
  showEdges,
  setShowEdges,
  selectedId,
  setSelectedId,
  structureOptions,
  materials,
}: MobileControlsProps): React.ReactElement {
  const [isOpen, setIsOpen] = useState(false);

  return (
    <>
      <button
        onClick={(): void => setIsOpen(true)}
        className="glass-card pointer-events-auto p-3"
        aria-label="Open settings"
      >
        <svg
          className="text-text-primary h-6 w-6"
          fill="none"
          viewBox="0 0 24 24"
          stroke="currentColor"
        >
          <path
            strokeLinecap="round"
            strokeLinejoin="round"
            strokeWidth={2}
            d="M10.325 4.317c.426-1.756 2.924-1.756 3.35 0a1.724 1.724 0 002.573 1.066c1.543-.94 3.31.826 2.37 2.37a1.724 1.724 0 001.065 2.572c1.756.426 1.756 2.924 0 3.35a1.724 1.724 0 00-1.066 2.573c.94 1.543-.826 3.31-2.37 2.37a1.724 1.724 0 00-2.572 1.065c-.426 1.756-2.924 1.756-3.35 0a1.724 1.724 0 00-2.573-1.066c-1.543.94-3.31-.826-2.37-2.37a1.724 1.724 0 00-1.065-2.572c-1.756-.426-1.756-2.924 0-3.35a1.724 1.724 0 001.066-2.573c-.94-1.543.826-3.31 2.37-2.37.996.608 2.296.07 2.572-1.065z"
          />
          <path
            strokeLinecap="round"
            strokeLinejoin="round"
            strokeWidth={2}
            d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"
          />
        </svg>
      </button>

      {isOpen && (
        <div className="fixed inset-0 z-50 flex items-end justify-center bg-black/70 p-4">
          <div className="glass-card pointer-events-auto w-full max-w-sm p-6">
            <div className="mb-4 flex items-center justify-between">
              <h3 className="text-text-primary font-semibold">Parameters</h3>
              <button
                onClick={(): void => setIsOpen(false)}
                className="text-text-muted hover:text-text-primary"
                aria-label="Close settings"
              >
                <svg className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth={2}
                    d="M6 18L18 6M6 6l12 12"
                  />
                </svg>
              </button>
            </div>

            <div className="space-y-6">
              <div>
                <label className="text-text-muted mb-2 block text-sm">Structure</label>
                <StructureSelector
                  options={structureOptions}
                  materials={materials}
                  selectedId={selectedId}
                  onChange={setSelectedId}
                  className="w-full"
                />
              </div>

              <div>
                <label className="text-text-muted mb-2 flex justify-between text-sm">
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
                <label className="text-text-muted mb-2 flex justify-between text-sm">
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

              <div className="flex items-center justify-between">
                <span className="text-text-muted text-sm">Show edges</span>
                <button
                  onClick={(): void => setShowEdges(!showEdges)}
                  className={`h-7 w-12 rounded-full transition-colors ${
                    showEdges ? 'bg-primary-mid' : 'bg-gray-600'
                  }`}
                >
                  <div
                    className={`h-6 w-6 rounded-full bg-white transition-transform ${
                      showEdges ? 'translate-x-5' : 'translate-x-0.5'
                    }`}
                  />
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
