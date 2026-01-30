export interface VectorFloat {
  size(): number;
  get(index: number): number;
  delete(): void;
}

export interface VectorInt {
  size(): number;
  get(index: number): number;
  delete(): void;
}

export interface VectorString {
  size(): number;
  get(index: number): string;
  delete(): void;
}

export interface WasmAPI {
  // Structure
  loadStructure(vaspContent: string): boolean;
  numAtoms(): number;
  getPositions(): VectorFloat;
  getAtomTypes(): VectorInt;
  getElements(): VectorString;
  getElementCounts(): VectorInt;
  getLatticeVectors(): VectorFloat;

  // Graph
  buildGraph(rCutoff: number, maxNeighbors: number): void;
  numEdges(): number;
  getEdgeSources(): VectorInt;
  getEdgeTargets(): VectorInt;
  getEdgeDistances(): VectorFloat;
  getEdgeDisplacements(): VectorFloat;

  delete(): void;
}

export interface DefectGNNModule {
  WasmAPI: new () => WasmAPI;
}

export type CreateDefectGNNModule = () => Promise<DefectGNNModule>;
