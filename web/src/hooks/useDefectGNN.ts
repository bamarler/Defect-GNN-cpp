'use client';

import { useState, useCallback, useRef, useEffect } from 'react';
import type {
  DefectGNNModule,
  WasmAPI,
  VectorFloat,
  VectorInt,
  VectorString,
} from '@/types/defect-gnn';

export interface StructureData {
  positions: Float32Array;
  atomTypes: Int32Array;
  elements: string[];
  elementCounts: number[];
  latticeVectors: Float32Array;
  numAtoms: number;
}

export interface GraphData {
  edgeSources: Int32Array;
  edgeTargets: Int32Array;
  edgeDistances: Float32Array;
  numEdges: number;
}

interface UseDefectGNNResult {
  isLoading: boolean;
  isReady: boolean;
  error: string | null;
  loadStructure: (vaspContent: string) => boolean;
  buildGraph: (rCutoff: number, maxNeighbors: number) => void;
  getStructureData: () => StructureData | null;
  getGraphData: () => GraphData | null;
}

function vectorFloatToArray(vec: VectorFloat): Float32Array {
  const arr = new Float32Array(vec.size());
  for (let i = 0; i < vec.size(); i++) {
    arr[i] = vec.get(i);
  }
  vec.delete();
  return arr;
}

function vectorIntToArray(vec: VectorInt): Int32Array {
  const arr = new Int32Array(vec.size());
  for (let i = 0; i < vec.size(); i++) {
    arr[i] = vec.get(i);
  }
  vec.delete();
  return arr;
}

function vectorStringToArray(vec: VectorString): string[] {
  const arr: string[] = [];
  for (let i = 0; i < vec.size(); i++) {
    arr.push(vec.get(i));
  }
  vec.delete();
  return arr;
}

export function useDefectGNN(): UseDefectGNNResult {
  const [isLoading, setIsLoading] = useState(true);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const moduleRef = useRef<DefectGNNModule | null>(null);
  const apiRef = useRef<WasmAPI | null>(null);

  useEffect(() => {
    let cancelled = false;

    async function loadWasmModule(): Promise<void> {
      try {
        const script = document.createElement('script');
        script.src = '/wasm/defect_gnn_viz.js';

        await new Promise<void>((resolve, reject) => {
          script.onload = (): void => resolve();
          script.onerror = (): void => reject(new Error('Failed to load WASM script'));
          document.head.appendChild(script);
        });

        if (cancelled) return;

        // @ts-expect-error - createDefectGNNModule is added globally by the script
        const wasmModule: DefectGNNModule = await window.createDefectGNNModule();

        if (cancelled) return;

        moduleRef.current = wasmModule;
        apiRef.current = new wasmModule.WasmAPI();
        setIsReady(true);
      } catch (err) {
        if (!cancelled) {
          setError(err instanceof Error ? err.message : 'Failed to load WASM module');
        }
      } finally {
        if (!cancelled) {
          setIsLoading(false);
        }
      }
    }

    loadWasmModule();

    return (): void => {
      cancelled = true;
      if (apiRef.current) {
        apiRef.current.delete();
        apiRef.current = null;
      }
    };
  }, []);

  const loadStructure = useCallback((vaspContent: string): boolean => {
    if (!apiRef.current) {
      console.error('WASM module not loaded');
      return false;
    }
    return apiRef.current.loadStructure(vaspContent);
  }, []);

  const buildGraph = useCallback((rCutoff: number, maxNeighbors: number): void => {
    if (!apiRef.current) {
      console.error('WASM module not loaded');
      return;
    }
    apiRef.current.buildGraph(rCutoff, maxNeighbors);
  }, []);

  const getStructureData = useCallback((): StructureData | null => {
    if (!apiRef.current) {
      return null;
    }

    const api = apiRef.current;
    const numAtoms = api.numAtoms();

    if (numAtoms === 0) {
      return null;
    }

    return {
      positions: vectorFloatToArray(api.getPositions()),
      atomTypes: vectorIntToArray(api.getAtomTypes()),
      elements: vectorStringToArray(api.getElements()),
      elementCounts: Array.from(vectorIntToArray(api.getElementCounts())),
      latticeVectors: vectorFloatToArray(api.getLatticeVectors()),
      numAtoms,
    };
  }, []);

  const getGraphData = useCallback((): GraphData | null => {
    if (!apiRef.current) {
      return null;
    }

    const api = apiRef.current;
    const numEdges = api.numEdges();

    if (numEdges === 0) {
      return null;
    }

    return {
      edgeSources: vectorIntToArray(api.getEdgeSources()),
      edgeTargets: vectorIntToArray(api.getEdgeTargets()),
      edgeDistances: vectorFloatToArray(api.getEdgeDistances()),
      numEdges,
    };
  }, []);

  return {
    isLoading,
    isReady,
    error,
    loadStructure,
    buildGraph,
    getStructureData,
    getGraphData,
  };
}
