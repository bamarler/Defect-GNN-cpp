// Type definitions for the WASM module

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
  loadStructure(vaspContent: string): boolean;
  buildGraph(rCutoff: number, maxNeighbors: number): void;
  numAtoms(): number;
  getPositions(): VectorFloat;
  getAtomTypes(): VectorInt;
  getElements(): VectorString;
  getElementCounts(): VectorInt;
  getLatticeVectors(): VectorFloat;
  numEdges(): number;
  getEdgeSources(): VectorInt;
  getEdgeTargets(): VectorInt;
  getEdgeDistances(): VectorFloat;
  delete(): void;
}

export interface DefectGNNModule {
  WasmAPI: new () => WasmAPI;
}

export type CreateDefectGNNModule = () => Promise<DefectGNNModule>;
