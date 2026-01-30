interface FooterProps {
  className?: string;
}

export function Footer({ className = '' }: FooterProps): React.ReactElement {
  return (
    <footer className={`text-text-muted text-center text-sm ${className}`}>
      <p className="mb-2">
        Based on{' '}
        <a
          href="https://doi.org/10.1021/acs.chemmater.4c03028"
          target="_blank"
          rel="noopener noreferrer"
          className="text-accent-primary hover:text-accent-hover transition-colors"
        >
          Fang &amp; Yan, <em>Chem. Mater.</em> 2025
        </a>
      </p>
      <p>
        Built by{' '}
        <a
          href="https://github.com/bamarler"
          target="_blank"
          rel="noopener noreferrer"
          className="text-primary-light hover:text-primary-mid transition-colors"
        >
          Ben Marler
        </a>
      </p>
    </footer>
  );
}
