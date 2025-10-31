"""Utilities to fetch and cache geographic information for the campus polygon."""

from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache
from typing import List, Sequence, Tuple

import geopandas as gpd
import osmnx as ox


@dataclass(frozen=True)
class LocationGeometry:
    """Container with the polygon and its derived coordinates."""

    polygon: gpd.GeoSeries
    coordinates: List[Tuple[float, float]]
    centroid: Tuple[float, float]


@lru_cache(maxsize=1)
def load_location_geometry(location_name: str) -> LocationGeometry:
    """Return the geometry for ``location_name`` from OpenStreetMap.

    The result is cached to avoid repeated calls to the geocoding service.
    """

    gdf: gpd.GeoDataFrame = ox.geocode_to_gdf(location_name)
    polygon = gdf.geometry.iloc[0]
    coords: Sequence[Tuple[float, float]] = list(polygon.exterior.coords)  # type: ignore[attr-defined]
    centroid_point = polygon.centroid
    centroid = (centroid_point.y, centroid_point.x)
    return LocationGeometry(polygon=gdf.geometry, coordinates=list(coords), centroid=centroid)
