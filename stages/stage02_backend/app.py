"""Stage 2: FastAPI backend that accepts live location updates."""

from __future__ import annotations

from datetime import datetime
from typing import List, Optional

from fastapi import FastAPI, Query
from fastapi.responses import HTMLResponse
import pandas as pd

from app_core import (
    LOCATION_NAME,
    LocationRecord,
    LocationStore,
    build_map,
    load_location_geometry,
)

from .schemas import LocationFilter, LocationIn, LocationOut

app = FastAPI(title="Location Tracker", version="0.1.0")

store = LocationStore()
geometry = load_location_geometry(LOCATION_NAME)

# Datos de ejemplo para facilitar las primeras pruebas.
store.bulk_add(
    [
        LocationRecord(latitude=3.3415, longitude=-76.5295, label="Punto 1", source="seed"),
        LocationRecord(latitude=3.3420, longitude=-76.5288, label="Punto 2", source="seed"),
    ]
)


@app.get("/locations", response_model=List[LocationOut])
def list_locations(
    start: Optional[datetime] = Query(None, description="ISO timestamp"),
    end: Optional[datetime] = Query(None, description="ISO timestamp"),
) -> List[LocationOut]:
    filters = LocationFilter(start=start, end=end)
    df = store.filter_by_date(filters.start, filters.end)
    records = df.to_dict(orient="records") if not df.empty else []
    result: List[LocationOut] = []
    for record in records:
        created_at = record.get("created_at")
        if isinstance(created_at, pd.Timestamp):
            created_at = created_at.to_pydatetime()
        result.append(
            LocationOut(
                latitude=record.get("latitude"),
                longitude=record.get("longitude"),
                label=record.get("label", ""),
                source=record.get("source", "api"),
                created_at=created_at,
            )
        )
    return result


@app.post("/locations", response_model=LocationOut, status_code=201)
def create_location(payload: LocationIn) -> LocationOut:
    record = LocationRecord(
        latitude=payload.latitude,
        longitude=payload.longitude,
        label=payload.label,
        source=payload.source,
    )
    store.add(record)
    return LocationOut.from_record(record)


@app.get("/map", response_class=HTMLResponse)
def render_map(
    start: Optional[datetime] = Query(None, description="ISO timestamp"),
    end: Optional[datetime] = Query(None, description="ISO timestamp"),
) -> HTMLResponse:
    filters = LocationFilter(start=start, end=end)
    df = store.filter_by_date(filters.start, filters.end)
    map_obj = build_map(geometry, df)
    return HTMLResponse(map_obj.get_root().render())


@app.post("/reset", status_code=204)
def reset_store() -> None:
    store.clear()
