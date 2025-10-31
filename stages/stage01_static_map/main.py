"""Stage 1: basic modular script that renders the campus and manual points."""

from __future__ import annotations

from app_core import LOCATION_NAME, LocationRecord, LocationStore, build_map, load_location_geometry


EXAMPLE_POINTS = [
    LocationRecord(latitude=3.3415, longitude=-76.5295, label="Punto 1"),
    LocationRecord(latitude=3.3420, longitude=-76.5288, label="Punto 2"),
]


def main(output_path: str = "stage1_map.html") -> None:
    geometry = load_location_geometry(LOCATION_NAME)
    store = LocationStore()
    store.bulk_add(EXAMPLE_POINTS)
    map_obj = build_map(geometry, store.dataframe())
    map_obj.save(output_path)
    print(f"Mapa renderizado en {output_path}")


if __name__ == "__main__":
    main()
