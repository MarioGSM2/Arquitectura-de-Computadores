"""Stage 3: Streamlit dashboard consuming the FastAPI backend."""

from __future__ import annotations

import datetime as dt
import os
from typing import Dict, Optional
from zoneinfo import ZoneInfo

import pandas as pd
import requests
import streamlit as st

DEFAULT_API_URL = os.getenv("TRACKER_API_URL", "http://localhost:8000")


def _build_params(start: Optional[dt.datetime], end: Optional[dt.datetime]) -> Dict[str, str]:
    params: Dict[str, str] = {}
    if start:
        params["start"] = start.isoformat()
    if end:
        params["end"] = end.isoformat()
    return params


def fetch_locations(start: Optional[dt.datetime], end: Optional[dt.datetime]) -> pd.DataFrame:
    params = _build_params(start, end)
    response = requests.get(f"{DEFAULT_API_URL}/locations", params=params, timeout=10)
    response.raise_for_status()
    data = response.json()
    if not data:
        return pd.DataFrame(columns=["latitude", "longitude", "label", "source", "created_at"])
    df = pd.DataFrame(data)
    df["created_at"] = pd.to_datetime(df["created_at"])
    return df


def fetch_map_html(start: Optional[dt.datetime], end: Optional[dt.datetime]) -> str:
    params = _build_params(start, end)
    response = requests.get(f"{DEFAULT_API_URL}/map", params=params, timeout=10)
    response.raise_for_status()
    return response.text


def submit_point(latitude: float, longitude: float, label: str, source: str = "dashboard") -> None:
    payload = {
        "latitude": latitude,
        "longitude": longitude,
        "label": label,
        "source": source,
    }
    response = requests.post(f"{DEFAULT_API_URL}/locations", json=payload, timeout=10)
    response.raise_for_status()


st.set_page_config(page_title="Rastreador Javeriana", layout="wide")
st.title("Rastreo de ubicaciones en campus")
st.caption(
    "Dashboard construido en etapas: mapa dinámico, API en tiempo real y tabla de históricos."
)

with st.sidebar:
    st.header("Configuración de vista")
    range_option = st.selectbox(
        "Rango de historial",
        options=(
            "Última hora",
            "Últimas 24 horas",
            "Todo",
            "Personalizado",
        ),
    )

    now = dt.datetime.utcnow()
    start_dt: Optional[dt.datetime] = None
    end_dt: Optional[dt.datetime] = None

    if range_option == "Última hora":
        start_dt = now - dt.timedelta(hours=1)
        end_dt = now
    elif range_option == "Últimas 24 horas":
        start_dt = now - dt.timedelta(days=1)
        end_dt = now
    elif range_option == "Personalizado":
        start_date = st.date_input("Fecha inicio", value=now.date())
        start_time = st.time_input("Hora inicio", value=dt.time(hour=0, minute=0))
        end_date = st.date_input("Fecha fin", value=now.date())
        end_time = st.time_input("Hora fin", value=dt.time(hour=23, minute=59))
        start_dt = dt.datetime.combine(start_date, start_time)
        end_dt = dt.datetime.combine(end_date, end_time)

    st.divider()
    st.subheader("Agregar punto manual")
    with st.form("manual_point"):
        lat = st.number_input("Latitud", value=3.3415, format="%.6f")
        lon = st.number_input("Longitud", value=-76.5295, format="%.6f")
        label = st.text_input("Etiqueta", value="Punto manual")
        submitted = st.form_submit_button("Enviar")
        if submitted:
            try:
                submit_point(lat, lon, label)
                st.success("Punto registrado correctamente")
            except requests.RequestException as exc:
                st.error(f"No fue posible registrar el punto: {exc}")


def _format_dataframe(df: pd.DataFrame) -> pd.DataFrame:
    if df.empty:
        return df
    df = df.copy()
    bogota_tz = ZoneInfo("America/Bogota")
    created_at_series = df["created_at"]
    if created_at_series.dt.tz is None:
        created_at_series = created_at_series.dt.tz_localize("UTC")
    else:
        created_at_series = created_at_series.dt.tz_convert("UTC")
    df["created_at"] = created_at_series.dt.tz_convert(bogota_tz)
    df = df.rename(
        columns={
            "latitude": "Latitud",
            "longitude": "Longitud",
            "label": "Etiqueta",
            "source": "Fuente",
            "created_at": "Fecha (Bogotá)",
        }
    )
    return df


try:
    locations_df = fetch_locations(start_dt, end_dt)
    map_html = fetch_map_html(start_dt, end_dt)
except requests.RequestException as exc:
    st.error(
        "No se pudo conectar con el backend FastAPI. Asegúrate de ejecutar la etapa 2 "
        f"(`uvicorn stages.stage02_backend.app:app --reload`). Detalle: {exc}"
    )
else:
    col_map, col_table = st.columns((2, 1))
    with col_map:
        st.subheader("Mapa interactivo")
        st.components.v1.html(map_html, height=600)
    with col_table:
        st.subheader("Historial de ubicaciones")
        st.dataframe(_format_dataframe(locations_df), use_container_width=True)

    st.divider()
    st.subheader("Estadísticas rápidas")
    st.metric("Puntos visibles", len(locations_df))
    if not locations_df.empty:
        st.write("Último punto registrado:")
        last_point = locations_df.iloc[-1]
        st.write(
            f"{last_point['label'] or 'Sin etiqueta'} - "
            f"({last_point['latitude']:.5f}, {last_point['longitude']:.5f}) - "
            f"{last_point['created_at']}"
        )
