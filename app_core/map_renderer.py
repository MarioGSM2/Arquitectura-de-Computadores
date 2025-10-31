"""Helpers to build interactive folium maps for the dashboard and API."""

from __future__ import annotations

from typing import Iterable, Tuple

import folium
import pandas as pd

from .config import MAP_ZOOM
from .geo import LocationGeometry


def _add_polygon(map_obj: folium.Map, geometry: LocationGeometry) -> None:
    folium.Polygon(
        locations=[(lat, lon) for lon, lat in geometry.coordinates],
        color="red",
        fill=True,
        fill_opacity=0.3,
        popup="Zona Javeriana",
    ).add_to(map_obj)


def _add_points(map_obj: folium.Map, points: Iterable[Tuple[float, float, str]]) -> None:
    for index, (lat, lon, label) in enumerate(points, start=1):
        popup_text = label or f"Punto {index}"
        folium.Marker((lat, lon), popup=popup_text).add_to(map_obj)


def _add_polyline(map_obj: folium.Map, df: pd.DataFrame) -> None:
    if len(df) > 1:
        coords = list(zip(df["latitude"], df["longitude"]))
        folium.PolyLine(coords, color="blue", weight=2.5).add_to(map_obj)


def build_map(geometry: LocationGeometry, points_df: pd.DataFrame) -> folium.Map:
    """Render a folium map with the campus polygon and the provided points."""

    map_obj = folium.Map(location=geometry.centroid, zoom_start=MAP_ZOOM)
    _add_polygon(map_obj, geometry)
    if not points_df.empty:
        labels = points_df["label"] if "label" in points_df else [""] * len(points_df)
        _add_points(
            map_obj,
            zip(
                points_df["latitude"],
                points_df["longitude"],
                labels,
            ),
        )
        _add_polyline(map_obj, points_df)
    return map_obj
