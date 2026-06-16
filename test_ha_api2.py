import urllib.request
import json
import ssl
import sys

url = "https://axellum.freeboxos.fr:32768/api/states"
headers = {
    "Authorization": "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI3YzU3NGZiMGE4MDU0NjIyYmUyOGM0NDdkMTFjODg1NSIsImlhdCI6MTc3MzIxNDU3MCwiZXhwIjoyMDg4NTc0NTcwfQ.R8J7JWgOyUrvlbRfArwYebblgDn4jZu8FjS3BbjcIlQ",
    "Content-Type": "application/json"
}
ctx = ssl.create_default_context()
ctx.check_hostname = False
ctx.verify_mode = ssl.CERT_NONE

req = urllib.request.Request(url, headers=headers)
try:
    with urllib.request.urlopen(req, context=ctx) as response:
        data = json.loads(response.read().decode())
        
        with open("ha_dump.txt", "w", encoding="utf-8") as f:
            for e in data:
                uid = e['entity_id']
                if uid.startswith("weather.") or "meteo" in uid or "nextion" in uid or "saint_vincent" in uid:
                    f.write(f"{uid} == {e['state']}\n")
                    # Dump attributes for weather
                    if uid.startswith("weather."):
                        for k, v in e.get("attributes", {}).items():
                            if k != "forecast": # skip big block
                                f.write(f"  - {k}: {v}\n")
except Exception as e:
    print(f"Error: {e}")
