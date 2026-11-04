"""Licznik energii HAN (wM-Bus) - odczyt z ESP8266 + CC1101 przez /json."""

from __future__ import annotations

import asyncio
from datetime import timedelta
import logging
from typing import Any

import aiohttp

from homeassistant.config_entries import ConfigEntry
from homeassistant.const import CONF_HOST, CONF_SCAN_INTERVAL, Platform
from homeassistant.core import HomeAssistant
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .const import DEFAULT_SCAN_INTERVAL, DOMAIN

_LOGGER = logging.getLogger(__name__)
PLATFORMS = [Platform.SENSOR]

type LicznikConfigEntry = ConfigEntry[LicznikCoordinator]


async def fetch_json(hass: HomeAssistant, host: str) -> dict[str, Any]:
    """Pobiera biezacy stan z ESP."""
    session = async_get_clientsession(hass)
    async with asyncio.timeout(10):
        resp = await session.get(f"http://{host}/json")
        resp.raise_for_status()
        return await resp.json(content_type=None)


class LicznikCoordinator(DataUpdateCoordinator[dict[str, Any]]):
    """Odpytuje ESP co scan_interval sekund."""

    def __init__(self, hass: HomeAssistant, entry: LicznikConfigEntry) -> None:
        super().__init__(
            hass,
            _LOGGER,
            config_entry=entry,
            name=DOMAIN,
            update_interval=timedelta(
                seconds=entry.options.get(CONF_SCAN_INTERVAL, DEFAULT_SCAN_INTERVAL)
            ),
        )
        self.host: str = entry.data[CONF_HOST]

    async def _async_update_data(self) -> dict[str, Any]:
        try:
            return await fetch_json(self.hass, self.host)
        except (aiohttp.ClientError, TimeoutError, ValueError) as err:
            raise UpdateFailed(f"Brak odpowiedzi z {self.host}: {err}") from err


async def async_setup_entry(hass: HomeAssistant, entry: LicznikConfigEntry) -> bool:
    coordinator = LicznikCoordinator(hass, entry)
    await coordinator.async_config_entry_first_refresh()
    entry.runtime_data = coordinator
    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)
    entry.async_on_unload(entry.add_update_listener(_async_reload))
    return True


async def _async_reload(hass: HomeAssistant, entry: LicznikConfigEntry) -> None:
    await hass.config_entries.async_reload(entry.entry_id)


async def async_unload_entry(hass: HomeAssistant, entry: LicznikConfigEntry) -> bool:
    return await hass.config_entries.async_unload_platforms(entry, PLATFORMS)
