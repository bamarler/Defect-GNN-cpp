'use client';

import { useEffect, useRef } from 'react';

interface PipelineModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export function PipelineModal({ isOpen, onClose }: PipelineModalProps): React.ReactElement | null {
  const modalRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    function handleEscape(e: KeyboardEvent): void {
      if (e.key === 'Escape') {
        onClose();
      }
    }

    function handleClickOutside(e: MouseEvent): void {
      if (modalRef.current && !modalRef.current.contains(e.target as Node)) {
        onClose();
      }
    }

    if (isOpen) {
      document.addEventListener('keydown', handleEscape);
      document.addEventListener('mousedown', handleClickOutside);
      document.body.style.overflow = 'hidden';
    }

    return (): void => {
      document.removeEventListener('keydown', handleEscape);
      document.removeEventListener('mousedown', handleClickOutside);
      document.body.style.overflow = '';
    };
  }, [isOpen, onClose]);

  if (!isOpen) {
    return null;
  }

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/70 p-4">
      <div ref={modalRef} className="glass-card max-h-[80vh] w-full max-w-2xl overflow-y-auto p-6">
        <div className="mb-4 flex items-center justify-between">
          <h2 className="text-gradient text-2xl font-bold">Data Pipeline</h2>
          <button
            onClick={onClose}
            className="text-text-muted hover:text-text-primary transition-colors"
            aria-label="Close modal"
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

        <div className="text-text-primary space-y-4 text-sm leading-relaxed">
          <section>
            <h3 className="text-primary-light mb-2 font-semibold">Overview</h3>
            <p className="text-text-muted">
              This tool visualizes crystal structures and their graph representations used for
              predicting defect formation energies. The pipeline transforms raw atomic coordinates
              into a format suitable for Graph Neural Networks (GNNs).
            </p>
          </section>

          <section>
            <h3 className="text-primary-light mb-2 font-semibold">1. Structure Parsing</h3>
            <p className="text-text-muted">
              Crystal structures are read from VASP POSCAR files, which contain lattice vectors and
              fractional atomic coordinates. These are converted to Cartesian coordinates for
              distance calculations.
            </p>
          </section>

          <section>
            <h3 className="text-primary-light mb-2 font-semibold">
              2. Periodic Boundary Conditions
            </h3>
            <p className="text-text-muted">
              Crystals repeat infinitely in all directions. To find neighbors accurately, we apply
              Periodic Boundary Conditions (PBC) using the minimum image convention. This ensures
              atoms near cell boundaries correctly detect neighbors in adjacent periodic images.
            </p>
          </section>

          <section>
            <h3 className="text-primary-light mb-2 font-semibold">3. Neighbor List Construction</h3>
            <p className="text-text-muted">
              A KD-tree spatial index efficiently finds atoms within the cutoff radius (r
              <sub>cutoff</sub>). Each atom connects to its nearest neighbors up to a maximum count.
              These connections become edges in the graph.
            </p>
          </section>

          <section>
            <h3 className="text-primary-light mb-2 font-semibold">4. Edge Features</h3>
            <p className="text-text-muted">
              Interatomic distances are encoded using Gaussian Radial Basis Functions (RBF). This
              expands each distance into a vector of Gaussian responses centered at regular
              intervals, giving the neural network a richer representation of atomic spacing.
            </p>
          </section>

          <section>
            <h3 className="text-primary-light mb-2 font-semibold">5. Graph Assembly</h3>
            <p className="text-text-muted">
              The final crystal graph contains node features (element embeddings), edge indices
              (neighbor connections), and edge attributes (RBF-encoded distances). This format feeds
              directly into graph convolution layers.
            </p>
          </section>

          <section className="border-primary-dark rounded-md border bg-black/20 p-3">
            <h3 className="text-accent-primary mb-2 font-semibold">Key Parameters</h3>
            <ul className="text-text-muted list-inside list-disc space-y-1">
              <li>
                <strong>r_cutoff</strong>: Maximum distance for neighbor connections (default: 10 Å)
              </li>
              <li>
                <strong>max_neighbors</strong>: Maximum edges per atom (default: 20)
              </li>
            </ul>
          </section>
        </div>
      </div>
    </div>
  );
}
