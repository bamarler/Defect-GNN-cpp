'use client';

import { useState, useEffect, useCallback } from 'react';
import Link from 'next/link';
import { useDefectGNN } from '@/hooks/useDefectGNN';
import { CrystalViewer } from '@/components/CrystalViewer';
import { AtomLegend } from '@/components/AtomLegend';
import { ControlsPanel } from '@/components/ControlsPanel';
import { MobileControls } from '@/components/MobileControls';
import {
  loadManifest,
  getStructureOptions,
  loadStructureFile,
  type StructureOption,
  type Material,
} from '@/lib/structure-loader';
import type { StructureData, GraphData } from '@/hooks/useDefectGNN';

export default function Visualizer(): React.ReactElement {
  const { isLoading, isReady, error, loadStructure, buildGraph, getStructureData, getGraphData } =
    useDefectGNN();

  const [structureData, setStructureData] = useState<StructureData | null>(null);
  const [graphData, setGraphData] = useState<GraphData | null>(null);
  const [showEdges, setShowEdges] = useState(false);
  const [rCutoff, setRCutoff] = useState(5.0);
  const [maxNeighbors, setMaxNeighbors] = useState(12);

  const [materials, setMaterials] = useState<Material[]>([]);
  const [structureOptions, setStructureOptions] = useState<StructureOption[]>([]);
  const [selectedId, setSelectedId] = useState<string>('');
  const [isLoadingStructure, setIsLoadingStructure] = useState(false);

  // Load manifest on mount
  useEffect(() => {
    loadManifest()
      .then((manifest) => {
        setMaterials(manifest.materials);
        const options = getStructureOptions(manifest);
        setStructureOptions(options);
        if (options.length > 0) {
          setSelectedId(options[0].id);
        }
      })
      .catch((err) => {
        console.error('Failed to load manifest:', err);
      });
  }, []);

  // Load structure when selection changes
  const loadSelectedStructure = useCallback(async (): Promise<void> => {
    if (!isReady || !selectedId) {
      return;
    }

    const option = structureOptions.find((o) => o.id === selectedId);
    if (!option) {
      return;
    }

    setIsLoadingStructure(true);

    try {
      const vaspContent = await loadStructureFile(option.filename);
      const success = loadStructure(vaspContent);

      if (success) {
        buildGraph(rCutoff, maxNeighbors);
        setStructureData(getStructureData());
        setGraphData(getGraphData());
      }
    } catch (err) {
      console.error('Failed to load structure:', err);
    } finally {
      setIsLoadingStructure(false);
    }
  }, [
    isReady,
    selectedId,
    structureOptions,
    loadStructure,
    buildGraph,
    getStructureData,
    getGraphData,
    rCutoff,
    maxNeighbors,
  ]);

  useEffect(() => {
    loadSelectedStructure();
  }, [loadSelectedStructure]);

  // Rebuild graph when parameters change
  useEffect(() => {
    if (!isReady || !structureData) {
      return;
    }

    buildGraph(rCutoff, maxNeighbors);
    setGraphData(getGraphData());
  }, [rCutoff, maxNeighbors, isReady, structureData, buildGraph, getGraphData]);

  const showLoading = isLoading || isLoadingStructure;

  const selectedOption = structureOptions.find((o) => o.id === selectedId);

  return (
    <div className="relative h-screen w-screen overflow-hidden bg-black">
      <CrystalViewer structureData={structureData} graphData={graphData} showEdges={showEdges} />

      {/* Back button */}
      <div className="pointer-events-none absolute left-4 top-4 z-10">
        <Link
          href="/"
          className="glass-card text-text-muted hover:text-text-primary pointer-events-auto flex items-center gap-2 px-3 py-2 text-sm transition-colors"
        >
          <svg className="h-4 w-4" fill="none" viewBox="0 0 24 24" stroke="currentColor">
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth={2}
              d="M15 19l-7-7 7-7"
            />
          </svg>
          Back
        </Link>
      </div>

      {/* Status indicator */}
      {(showLoading || error) && (
        <div className="absolute left-1/2 top-4 z-10 -translate-x-1/2">
          <div className="glass-card px-4 py-2 text-sm">
            {showLoading && <span className="text-text-muted">Loading...</span>}
            {error && <span className="text-red-400">Error: {error}</span>}
          </div>
        </div>
      )}

      {/* Legend - bottom left */}
      {structureData && (
        <div className="pointer-events-none absolute bottom-4 left-4 z-10">
          <AtomLegend
            elements={structureData.elements}
            elementCounts={structureData.elementCounts}
          />
        </div>
      )}

      {/* Controls - desktop */}
      <div className="pointer-events-none absolute bottom-4 right-4 z-10 hidden md:block">
        <ControlsPanel
          rCutoff={rCutoff}
          setRCutoff={setRCutoff}
          maxNeighbors={maxNeighbors}
          setMaxNeighbors={setMaxNeighbors}
          showEdges={showEdges}
          setShowEdges={setShowEdges}
          selectedId={selectedId}
          setSelectedId={setSelectedId}
          structureOptions={structureOptions}
          materials={materials}
        />
      </div>

      {/* Controls - mobile */}
      <div className="pointer-events-none absolute bottom-4 right-4 z-10 md:hidden">
        <MobileControls
          rCutoff={rCutoff}
          setRCutoff={setRCutoff}
          maxNeighbors={maxNeighbors}
          setMaxNeighbors={setMaxNeighbors}
          showEdges={showEdges}
          setShowEdges={setShowEdges}
          selectedId={selectedId}
          setSelectedId={setSelectedId}
          structureOptions={structureOptions}
          materials={materials}
        />
      </div>

      {/* Structure label - mobile */}
      {selectedOption && (
        <div className="pointer-events-none absolute bottom-20 left-1/2 z-10 -translate-x-1/2 md:hidden">
          <div className="glass-card px-4 py-2">
            <span className="text-text-primary text-sm font-semibold">
              {selectedOption.materialName}
            </span>
            <span className="text-text-muted text-sm">
              {' '}
              (
              {selectedOption.type === 'pristine'
                ? 'Pristine'
                : `Defect ${selectedOption.id.split('-').pop()}`}
              )
            </span>
          </div>
        </div>
      )}
    </div>
  );
}
