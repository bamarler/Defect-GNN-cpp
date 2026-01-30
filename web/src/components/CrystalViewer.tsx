'use client';

import { useEffect, useRef } from 'react';
import type { StructureData, GraphData } from '@/hooks/useDefectGNN';
import { getAtomicNumber, getColorByElementIndex, getRadiusByAtomicNumber } from '@/lib/atoms';

interface Viewer {
  addSphere: (spec: SphereSpec) => void;
  addCylinder: (spec: CylinderSpec) => void;
  setBackgroundColor: (color: string) => void;
  zoomTo: () => void;
  zoom: (factor: number) => void;
  render: () => void;
  clear: () => void;
}

interface SphereSpec {
  center: { x: number; y: number; z: number };
  radius: number;
  color: string;
}

interface CylinderSpec {
  start: { x: number; y: number; z: number };
  end: { x: number; y: number; z: number };
  radius: number;
  color: string;
  fromCap?: boolean;
  toCap?: boolean;
}

interface $3Dmol {
  createViewer: (element: HTMLElement, config: { backgroundColor: string }) => Viewer;
}

declare global {
  interface Window {
    $3Dmol: $3Dmol;
  }
}

interface CrystalViewerProps {
  structureData: StructureData | null;
  graphData: GraphData | null;
  showEdges: boolean;
}

export function CrystalViewer({
  structureData,
  graphData,
  showEdges,
}: CrystalViewerProps): React.ReactElement {
  const containerRef = useRef<HTMLDivElement>(null);
  const viewerRef = useRef<Viewer | null>(null);
  const scriptLoadedRef = useRef(false);

  useEffect(() => {
    if (scriptLoadedRef.current || typeof window === 'undefined') {
      return;
    }

    const script = document.createElement('script');
    script.src = 'https://3dmol.org/build/3Dmol-min.js';
    script.async = true;
    script.onload = (): void => {
      scriptLoadedRef.current = true;
      if (containerRef.current && window.$3Dmol) {
        viewerRef.current = window.$3Dmol.createViewer(containerRef.current, {
          backgroundColor: '0x000000',
        });
      }
    };
    document.head.appendChild(script);
  }, []);

  useEffect(() => {
    if (!viewerRef.current || !structureData) {
      return;
    }

    const viewer = viewerRef.current;
    viewer.clear();

    const { positions, atomTypes, elements } = structureData;

    for (let i = 0; i < structureData.numAtoms; i++) {
      const x = positions[i * 3];
      const y = positions[i * 3 + 1];
      const z = positions[i * 3 + 2];
      const elementIndex = atomTypes[i];
      const element = elements[elementIndex];
      const atomicNum = getAtomicNumber(element);

      viewer.addSphere({
        center: { x, y, z },
        radius: getRadiusByAtomicNumber(atomicNum),
        color: getColorByElementIndex(elementIndex),
      });
    }

    if (showEdges && graphData) {
      const { edgeSources, edgeTargets } = graphData;

      for (let i = 0; i < graphData.numEdges; i++) {
        const src = edgeSources[i];
        const tgt = edgeTargets[i];

        if (src >= tgt) {
          continue;
        }

        viewer.addCylinder({
          start: {
            x: positions[src * 3],
            y: positions[src * 3 + 1],
            z: positions[src * 3 + 2],
          },
          end: {
            x: positions[tgt * 3],
            y: positions[tgt * 3 + 1],
            z: positions[tgt * 3 + 2],
          },
          radius: 0.025,
          color: '#4a4a4a',
          fromCap: false,
          toCap: false,
        });
      }
    }

    viewer.zoomTo();
    viewer.zoom(0.8);
    viewer.render();
  }, [structureData, graphData, showEdges]);

  return <div ref={containerRef} className="h-full w-full" style={{ position: 'relative' }} />;
}
