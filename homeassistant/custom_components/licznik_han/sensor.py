"""Encje licznika: energia, moc, napiecia, diagnostyka."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from typing import Any

from homeassistant.components.sensor import (
    SensorDeviceClass,
    SensorEntity,
    SensorEntityDescription,
    SensorStateClass,
)
from homeassistant.const import (
    SIGNAL_STRENGTH_DECIBELS_MILLIWATT,
    EntityCategory,
    UnitOfElectricPotential,
    UnitOfEnergy,
    UnitOfPower,
    UnitOfTime,
)
from homeassistant.core import HomeAssistant
from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from . import LicznikConfigEntry, LicznikCoordinator
from .const import DOMAIN, STALE_AFTER_S


@dataclass(frozen=True, kw_only=True)
class LicznikSensorDescription(SensorEntityDescription):
    value_fn: Callable[[dict[str, Any]], Any]
    from_meter: bool = True  # wartosc z ramki licznika (niedostepna, gdy brak swiezego odczytu)
    optional: bool = False   # tworzona tylko, jesli licznik ja wysyla


def _m(key: str) -> Callable[[dict[str, Any]], Any]:
    return lambda d: (d.get("meter") or {}).get(key)


def _net(d: dict[str, Any]) -> float | None:
    m = d.get("meter") or {}
    imp, exp = m.get("power_import_w"), m.get("power_export_w")
    if imp is None and exp is None:
        return None
    return (imp or 0) - (exp or 0)


def _energy(key: str, name: str, optional: bool = False) -> LicznikSensorDescription:
    return LicznikSensorDescription(
        key=key,
        name=name,
        value_fn=_m(key),
        optional=optional,
        device_class=SensorDeviceClass.ENERGY,
        state_class=SensorStateClass.TOTAL_INCREASING,
        native_unit_of_measurement=UnitOfEnergy.KILO_WATT_HOUR,
        suggested_display_precision=3,
    )


def _power(key: str, name: str, value_fn=None, **kw) -> LicznikSensorDescription:
    return LicznikSensorDescription(
        key=key,
        name=name,
        value_fn=value_fn or _m(key),
        device_class=SensorDeviceClass.POWER,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfPower.WATT,
        suggested_display_precision=0,
        **kw,
    )


SENSORS: tuple[LicznikSensorDescription, ...] = (
    _energy("import_kwh", "Energia pobrana"),
    _energy("export_kwh", "Energia oddana"),
    *(_energy(f"import_t{t}_kwh", f"Energia pobrana T{t}", True) for t in range(1, 5)),
    *(_energy(f"export_t{t}_kwh", f"Energia oddana T{t}", True) for t in range(1, 5)),
    # przyrosty od poczatku doby / miesiaca (liczone w ESP z zapisanych stanow dobowych)
    _energy("today_import_kwh", "Pobór dziś"),
    _energy("month_import_kwh", "Pobór w tym miesiącu"),
    _energy("today_export_kwh", "Oddanie dziś", True),
    _energy("month_export_kwh", "Oddanie w tym miesiącu", True),
    LicznikSensorDescription(
        key="month_cost_pln",
        name="Koszt energii w tym miesiącu",
        value_fn=_m("month_cost_pln"),
        device_class=SensorDeviceClass.MONETARY,
        state_class=SensorStateClass.TOTAL,
        native_unit_of_measurement="PLN",
        suggested_display_precision=2,
    ),
    _power("power_import_w", "Moc pobierana"),
    _power("power_export_w", "Moc oddawana"),
    _power("power_net_w", "Moc netto", _net),
    _power("power_max_w", "Moc maksymalna", entity_category=EntityCategory.DIAGNOSTIC),
    _power("power_limit_w", "Próg mocy", entity_category=EntityCategory.DIAGNOSTIC),
    *(
        LicznikSensorDescription(
            key=f"voltage_l{p}",
            name=f"Napięcie L{p}",
            value_fn=_m(f"voltage_l{p}"),
            optional=True,
            device_class=SensorDeviceClass.VOLTAGE,
            state_class=SensorStateClass.MEASUREMENT,
            native_unit_of_measurement=UnitOfElectricPotential.VOLT,
            suggested_display_precision=1,
        )
        for p in range(1, 4)
    ),
    LicznikSensorDescription(
        key="rssi",
        name="Sygnał radiowy",
        value_fn=_m("rssi"),
        device_class=SensorDeviceClass.SIGNAL_STRENGTH,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=SIGNAL_STRENGTH_DECIBELS_MILLIWATT,
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    LicznikSensorDescription(
        key="age_s",
        name="Wiek odczytu",
        value_fn=_m("age_s"),
        from_meter=False,
        device_class=SensorDeviceClass.DURATION,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfTime.SECONDS,
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    LicznikSensorDescription(
        key="datetime",
        name="Czas licznika",
        value_fn=_m("datetime"),
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    LicznikSensorDescription(
        key="message",
        name="Komunikat licznika",
        value_fn=lambda d: _m("message")(d) or None,
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    LicznikSensorDescription(
        key="frames",
        name="Odebrane ramki",
        value_fn=lambda d: d.get("frames"),
        from_meter=False,
        state_class=SensorStateClass.TOTAL_INCREASING,
        entity_category=EntityCategory.DIAGNOSTIC,
        entity_registry_enabled_default=False,
    ),
    LicznikSensorDescription(
        key="crc_errors",
        name="Błędy CRC",
        value_fn=lambda d: d.get("crc_errors"),
        from_meter=False,
        state_class=SensorStateClass.TOTAL_INCREASING,
        entity_category=EntityCategory.DIAGNOSTIC,
        entity_registry_enabled_default=False,
    ),
)


async def async_setup_entry(
    hass: HomeAssistant, entry: LicznikConfigEntry, async_add_entities: AddEntitiesCallback
) -> None:
    coordinator = entry.runtime_data
    data = coordinator.data or {}
    async_add_entities(
        LicznikSensor(coordinator, entry, desc)
        for desc in SENSORS
        # opcjonalne (taryfy, fazy) tylko gdy licznik je wysyla; bez odczytu tworzymy wszystkie
        if not desc.optional or not data.get("meter") or desc.value_fn(data) is not None
    )


class LicznikSensor(CoordinatorEntity[LicznikCoordinator], SensorEntity):
    _attr_has_entity_name = True
    entity_description: LicznikSensorDescription

    def __init__(
        self, coordinator: LicznikCoordinator, entry: LicznikConfigEntry, desc: LicznikSensorDescription
    ) -> None:
        super().__init__(coordinator)
        self.entity_description = desc
        self._attr_unique_id = f"{entry.unique_id}_{desc.key}"
        meter = (coordinator.data or {}).get("meter") or {}
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, entry.unique_id)},
            name=entry.title,
            manufacturer=meter.get("manufacturer") or None,
            model="AMI – HAN wireless M-Bus",
            serial_number=meter.get("serial") or None,
            configuration_url=f"http://{coordinator.host}/",
        )

    @property
    def available(self) -> bool:
        if not super().available:
            return False
        if not self.entity_description.from_meter:
            return True
        meter = self.coordinator.data.get("meter")
        return meter is not None and meter.get("age_s", STALE_AFTER_S + 1) <= STALE_AFTER_S

    @property
    def native_value(self) -> Any:
        return self.entity_description.value_fn(self.coordinator.data)
