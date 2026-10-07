import csv
import sys
from pathlib import Path
from xml.sax.saxutils import escape


def parse_number(value):
    try:
        return float(value)
    except (ValueError, TypeError):
        return None


def csv_to_kml(csv_path, kml_path):
    points = []

    with open(csv_path, "r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)

        required = {"latitude", "longitude"}

        if not required.issubset(set(reader.fieldnames or [])):
            raise RuntimeError(
                f"CSV must contain columns: {required}"
            )

        for row in reader:
            lat = parse_number(row.get("latitude"))
            lon = parse_number(row.get("longitude"))

            if lat is None or lon is None:
                continue

            points.append({
                "lat": lat,
                "lon": lon,
                "timestamp": row.get("timestamp", ""),
                "altitude": row.get("altitude_m", ""),
                "speed": row.get("speed_kmph", ""),
                "satellites": row.get("satellites", ""),
                "hdop": row.get("hdop", ""),
            })

    if not points:
        raise RuntimeError("No valid GPS points found.")

    # Track coordinates: longitude,latitude,altitude
    coordinates = "\n".join(
        f"{p['lon']},{p['lat']},{p['altitude'] or 0}"
        for p in points
    )

    # Individual points
    placemarks = []

    for i, p in enumerate(points):
        description = (
            f"Time: {escape(str(p['timestamp']))}<br>"
            f"Latitude: {p['lat']}<br>"
            f"Longitude: {p['lon']}<br>"
            f"Altitude: {escape(str(p['altitude']))} m<br>"
            f"Speed: {escape(str(p['speed']))} km/h<br>"
            f"Satellites: {escape(str(p['satellites']))}<br>"
            f"HDOP: {escape(str(p['hdop']))}"
        )

        placemarks.append(f"""
        <Placemark>
            <name>GPS Point {i + 1}</name>
            <description><![CDATA[{description}]]></description>
            <Point>
                <coordinates>
                    {p['lon']},{p['lat']},{p['altitude'] or 0}
                </coordinates>
            </Point>
        </Placemark>
        """)

    kml = f"""<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>

    <name>{escape(csv_path.stem)}</name>

    <!-- Track style -->
    <Style id="trackStyle">
        <LineStyle>
            <color>ff0000ff</color>
            <width>5</width>
        </LineStyle>
    </Style>

    <!-- GPS track -->
    <Placemark>
        <name>GPS Track</name>
        <styleUrl>#trackStyle</styleUrl>

        <LineString>
            <tessellate>1</tessellate>
            <altitudeMode>absolute</altitudeMode>

            <coordinates>
                {coordinates}
            </coordinates>
        </LineString>
    </Placemark>

    <!-- Start -->
    <Placemark>
        <name>START</name>
        <description>Start of GPS track</description>
        <Point>
            <coordinates>
                {points[0]['lon']},{points[0]['lat']},{points[0]['altitude'] or 0}
            </coordinates>
        </Point>
    </Placemark>

    <!-- End -->
    <Placemark>
        <name>END</name>
        <description>End of GPS track</description>
        <Point>
            <coordinates>
                {points[-1]['lon']},{points[-1]['lat']},{points[-1]['altitude'] or 0}
            </coordinates>
        </Point>
    </Placemark>

    <!-- Individual GPS points -->
    {''.join(placemarks)}

</Document>
</kml>
"""

    with open(kml_path, "w", encoding="utf-8") as f:
        f.write(kml)

    print(f"Loaded {len(points)} GPS points.")
    print(f"Created: {kml_path}")


def main():
    if len(sys.argv) != 2:
        print("Usage:")
        print("  python plot_gps.py data1.csv")
        sys.exit(1)

    csv_path = Path(sys.argv[1]).resolve()

    if not csv_path.exists():
        print(f"ERROR: File not found: {csv_path}")
        sys.exit(1)

    kml_path = csv_path.with_suffix(".kml")

    try:
        csv_to_kml(csv_path, kml_path)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
