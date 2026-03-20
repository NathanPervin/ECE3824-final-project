import json
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_POST
from .models import CO2Reading
from django.shortcuts import render

@csrf_exempt
@require_POST
def log_co2(request):
    try:
        data = json.loads(request.body)
    except json.JSONDecodeError:
        return JsonResponse({"error": "Invalid JSON"}, status=400)

    if isinstance(data, dict):
        data = [data]

    if not isinstance(data, list):
        return JsonResponse({"error": "Expected a JSON object or array"}, status=400)

    try:
        readings = [
            CO2Reading(
                mode=entry["mode"],
                building=entry["building"],
                room_number=entry["room_number"],
                unix_timestamp=entry["unix_timestamp"],
                co2_ppm=entry["CO2_ppm"],
            )
            for entry in data
        ]
    except KeyError as e:
        return JsonResponse({"error": f"Missing field: {e}"}, status=400)

    CO2Reading.objects.bulk_create(readings)
    return JsonResponse({"success": True, "records_saved": len(readings)}, status=201)