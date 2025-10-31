"""Configuration constants used throughout the project."""

from __future__ import annotations

LOCATION_NAME: str = "Pontificia Universidad Javeriana, Cali, Colombia"
"""Default location that will be geocoded to create the campus polygon."""

MAP_ZOOM: int = 17
"""Default zoom level when rendering the folium map."""

POINT_HISTORY_LIMIT: int | None = None
"""Optional limit for the number of historical points kept in memory."""
