'use client';

import { useState } from 'react';
import Link from 'next/link';
import { PipelineModal } from '@/components/PipelineModal';
import { Footer } from '@/components/Footer';

export default function Home(): React.ReactElement {
  const [isPipelineModalOpen, setIsPipelineModalOpen] = useState(false);

  return (
    <main className="flex min-h-screen flex-col items-center justify-center p-8">
      <div className="max-w-2xl text-center">
        <h1 className="text-gradient mb-6 text-5xl font-bold">Defect GNN Visualizer</h1>

        <p className="text-text-muted mb-8 text-lg leading-relaxed">
          Explore crystal structures and their graph representations for predicting vacancy
          formation energies in perovskite materials. This tool visualizes the data pipeline that
          transforms atomic coordinates into inputs for Graph Neural Networks.
        </p>

        <div className="mb-8 flex flex-col items-center gap-4 sm:flex-row sm:justify-center">
          <Link
            href="/visualizer"
            className="bg-primary-mid hover:bg-primary-light rounded-lg px-8 py-3 font-semibold text-white transition-colors"
          >
            Launch Visualizer
          </Link>

          <button
            onClick={(): void => setIsPipelineModalOpen(true)}
            className="text-text-muted hover:text-text-primary border-text-muted hover:border-text-primary rounded-lg border px-8 py-3 font-semibold transition-colors"
          >
            How It Works
          </button>
        </div>

        <div className="glass-card mb-8 p-6 text-left">
          <h2 className="text-primary-light mb-3 text-lg font-semibold">About This Project</h2>
          <p className="text-text-muted mb-4 text-sm leading-relaxed">
            This is a C++ implementation of the defect formation energy prediction pipeline,
            compiled to WebAssembly for browser-based visualization. The approach combines graph
            neural networks with persistent homology features to achieve state-of-the-art accuracy.
          </p>
          <p className="text-text-muted text-sm leading-relaxed">
            <span className="text-accent-primary">Note:</span> The GNN training module is currently
            under development. This visualizer demonstrates the data preprocessing stages: structure
            parsing, neighbor list construction with Periodic Boundary Conditions, and graph
            assembly.
          </p>
        </div>

        <Footer />
      </div>

      <PipelineModal
        isOpen={isPipelineModalOpen}
        onClose={(): void => setIsPipelineModalOpen(false)}
      />
    </main>
  );
}
