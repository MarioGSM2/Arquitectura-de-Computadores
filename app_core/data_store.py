"""In-memory storage for current and historical location samples."""

from __future__ import annotations

from dataclasses import dataclass, asdict, field
from datetime import datetime
from threading import Lock
from typing import Iterable, List, Optional

import pandas as pd

from .config import POINT_HISTORY_LIMIT


@dataclass(frozen=True)
class LocationRecord:
    """Representation of a point gathered from the tracking system."""

    latitude: float
    longitude: float
    label: str = ""
    source: str = "manual"
    created_at: datetime = field(default_factory=datetime.utcnow)

    def to_dict(self) -> dict:
        data = asdict(self)
        data["created_at"] = self.created_at.isoformat()
        return data


class LocationStore:
    """Thread-safe container used by the API and dashboard."""

    def __init__(self) -> None:
        self._lock = Lock()
        self._data = pd.DataFrame(columns=["latitude", "longitude", "label", "source", "created_at"])

    def add(self, record: LocationRecord) -> None:
        """Append a record to the store, keeping the optional size limit."""

        with self._lock:
            row = pd.DataFrame([record.to_dict()])
            self._data = pd.concat([self._data, row], ignore_index=True)
            if POINT_HISTORY_LIMIT is not None and len(self._data) > POINT_HISTORY_LIMIT:
                self._data = self._data.iloc[-POINT_HISTORY_LIMIT :].reset_index(drop=True)

    def bulk_add(self, records: Iterable[LocationRecord]) -> None:
        for record in records:
            self.add(record)

    def dataframe(self) -> pd.DataFrame:
        """Return a copy of the stored data as a DataFrame sorted by timestamp."""

        with self._lock:
            if self._data.empty:
                return self._data.copy()
            df = self._data.copy()
        df["created_at"] = pd.to_datetime(df["created_at"])
        return df.sort_values("created_at")

    def filter_by_date(
        self, start: Optional[datetime] = None, end: Optional[datetime] = None
    ) -> pd.DataFrame:
        """Return the points within the provided date range."""

        df = self.dataframe()
        if start is not None:
            df = df[df["created_at"] >= start]
        if end is not None:
            df = df[df["created_at"] <= end]
        return df

    def latest(self, limit: int = 10) -> pd.DataFrame:
        df = self.dataframe()
        if df.empty:
            return df
        return df.tail(limit)

    def clear(self) -> None:
        with self._lock:
            self._data = self._data.iloc[0:0]
