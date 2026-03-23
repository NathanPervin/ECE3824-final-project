import json
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_POST
from .models import CO2Reading
from django.shortcuts import render
from django.conf import settings
import uuid

SECRET_TOKEN = settings.CO2_API_TOKEN
SESSION_GAP_INTERVAL = settings.SESSION_GAP_INTERVAL

@csrf_exempt
@require_POST
def log_co2(request):

    token = request.headers.get("Authorization")
    if token != f"Bearer {SECRET_TOKEN}":
        return JsonResponse({"error": "Unauthorized"}, status=401)
    
    try:
        data = json.loads(request.body)
    except json.JSONDecodeError:
        return JsonResponse({"error": "Invalid JSON"}, status=400)

    if isinstance(data, dict):
        data = [data]

    if not isinstance(data, list):
        return JsonResponse({"error": "Expected a JSON object or array"}, status=400)

    try:
        # check only the earliest timestamp (furthest in time from when its sent) 
        # in the batch against the last entry in the database
        first_entry = data[0]
        session_id = get_session_id(
            building=first_entry["building"],
            room_number=str(first_entry["room_number"]),
            mode=first_entry["mode"],
            unix_timestamp=first_entry["unix_timestamp"],
        )

        readings = [
            CO2Reading(
                mode=entry["mode"],
                building=entry["building"],
                room_number=entry["room_number"],
                unix_timestamp=entry["unix_timestamp"],
                co2_ppm=entry["CO2_ppm"],
                session_id=session_id,
            )
            for entry in data
        ]
    except KeyError as e:
        return JsonResponse({"error": f"Missing field: {e}"}, status=400)

    CO2Reading.objects.bulk_create(readings)
    return JsonResponse({"success": True, "records_saved": len(readings)}, status=201)

def get_session_id(building, room_number, mode, unix_timestamp):

    # get the most recently inserted timestamp in the database for 
    # the current building, room, and mode
    last_sample_info = (CO2Reading.objects
            .filter(building=building, room_number=room_number, mode=mode)
            .order_by('-unix_timestamp') # sort by most recent time first
            .values('unix_timestamp', 'session_id') # get these two values
            .first())  # only get the most recent row
    
    # if there is no other session in the same building and room number, its automatically a new session
    # otherwise if the time between data points exceeds the session gap, its also a new session
    if last_sample_info is None or (unix_timestamp - last_sample_info['unix_timestamp']) > SESSION_GAP_INTERVAL:
        return str(uuid.uuid4())  # new session
    
    # if above did not return, its not a new session, use the session id from the last database entry
    return last_sample_info['session_id']