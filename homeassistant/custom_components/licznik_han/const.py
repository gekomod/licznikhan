"""Stale integracji Licznik energii HAN."""

DOMAIN = "licznik_han"

DEFAULT_HOST = "licznik-han.local"
DEFAULT_SCAN_INTERVAL = 10  # s

# Po tylu sekundach bez nowej ramki z licznika encje licznika sa niedostepne
STALE_AFTER_S = 900
