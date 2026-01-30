import type { Metadata } from 'next';
import './globals.css';

export const metadata: Metadata = {
  title: 'Defect GNN Visualizer',
  description: 'Crystal structure and graph visualization',
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}): React.ReactElement {
  return (
    <html lang="en">
      <body>{children}</body>
    </html>
  );
}
