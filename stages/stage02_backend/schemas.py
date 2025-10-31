"""Pydantic models for the FastAPI backend."""

from __future__ import annotations

from datetime import datetime
from typing import Optional

from pydantic import BaseModel, Field, validator

from app_core import LocationRecord


class LocationIn(BaseModel):
    latitude: float = Field(..., ge=-90, le=90)
    longitude: float = Field(..., ge=-180, le=180)
    label: str = ""
    source: str = "api"


class LocationOut(LocationIn):
    created_at: datetime

    @classmethod
    def from_record(cls, record: LocationRecord) -> "LocationOut":
        return cls(
            latitude=record.latitude,
            longitude=record.longitude,
            label=record.label,
            source=record.source,
            created_at=record.created_at,
        )


class LocationFilter(BaseModel):
    start: Optional[datetime] = None
    end: Optional[datetime] = None

    @validator("end")
    def validate_range(cls, end: Optional[datetime], values: dict) -> Optional[datetime]:
        start = values.get("start")
        if start and end and end < start:
            raise ValueError("end must be greater than start")
        return end
