from django.shortcuts import render
from django.contrib.auth.decorators import login_required
from api.models import CO2Reading
#import time
from datetime import datetime, timezone
from django.db.models import Min, Max, Count

POST_INTERVAL = 300 # seconds. 5 minutes between POSTs

# Create your views here.
@login_required(login_url='/users/login_user')  # redirects here if not logged in
def co2_table(request):
    readings = CO2Reading.objects.order_by("-unix_timestamp")[:100]
    return render(request, "dashboard/co2_table.html", {"readings": readings})

@login_required(login_url='/users/login_user')  # redirects here if not logged in
def search(request):

    # get the building and room number from user's GET request after pressing search button
    building = request.GET.get('building', '').strip()
    room = request.GET.get('room', '').strip()

    results = None
    qs = CO2Reading.objects # query set to build the building and room filter
    
    # Add building and room number filters, one or the other is fine 
    if building:
        qs = qs.filter(building__icontains=building)
    if room:
        qs = qs.filter(room_number__icontains=room)
    
    # if nothing is entered for building or room, results will be None
    # and the html table won't be displayed, otherwise results
    # will have the database entries for user's filter
    if building or room:
        results = (qs.values('building', 'room_number') # return these three values
                    .distinct() # remove duplicates for the many data points
                    .order_by('building', 'room_number')) # sort by building, then room number

    return render(request, 'dashboard/search.html', {
        'results': results,
        'q_building': building,
        'q_room': room,
    })

@login_required(login_url='/users/login_user')  # redirects here if not logged in
def sessions(request, building, room):

    # get the filter fields entered by the user
    mode      = request.GET.get('mode', '')
    date_from = request.GET.get('date_from', '')
    date_to   = request.GET.get('date_to', '')

    # filter should start by containing the chosen building and room number
    qs = CO2Reading.objects.filter(building=building, room_number=room)

    if mode:
        qs = qs.filter(mode=mode)

    # Get the start and end date and apply them to the filter
    if date_from:
        try:
            dt = datetime.strptime(date_from, '%Y-%m-%d')
            qs = qs.filter(unix_timestamp__gte=int(dt.timestamp()))
        except ValueError: # only can occur if date in url is entered manually, UI entry via flatpickr are valid
            pass
    if date_to:
        try:
            dt = datetime.strptime(date_to, '%Y-%m-%d').replace(hour=23, minute=59, second=59)
            qs = qs.filter(unix_timestamp__lte=int(dt.timestamp()))
        except ValueError: # only can occur if date in url is entered manually, UI entry via flatpickr are valid
            pass

    raw_sessions = (qs
                    .values('mode', 'session_id') 
                    .annotate( # adds additional return key-values for the start time, end time, and number of samples
                        start=Min('unix_timestamp'),
                        end=Max('unix_timestamp'),
                        count=Count('id'),
                    )
                    .order_by('-start')) # sorts by most recent time

    sessions = []
    for session in raw_sessions:

        # subtract the start unix timestamp from the end unix timestamp
        # this gives the duration of the session in seconds
        total_seconds = session['end'] - session['start']

        # divide by 3600 seconds to get the number of hours, the remainer is the number of seconds
        session_duration_hours, remainder = divmod(total_seconds, 3600)

        # divide the number of seconds after the hours by 60 to find the number of minutes, 
        # the remainder is the number of seconds
        session_duration_minutes, session_duration_seconds = divmod(remainder, 60)

        # save current session + new info to sessions list
        sessions.append({
        'mode':        session['mode'],
        'session_id':  session['session_id'],
        'building':    building,
        'room_number': room,
        'start':       session['start'],
        'end':         session['end'],
        'count':       session['count'],
        'start_fmt':   datetime.fromtimestamp(session['start'], tz=timezone.utc).strftime('%Y-%m-%d %H:%M:%S'),
        'end_fmt':     datetime.fromtimestamp(session['end'], tz=timezone.utc).strftime('%Y-%m-%d %H:%M:%S'),
        'duration':    f"{int(session_duration_hours)}h {int(session_duration_minutes)}m {int(session_duration_seconds)}s",
        })

    return render(request, 'dashboard/sessions.html', {
        'building':        building,
        'room':            room,
        'sessions':        sessions,
        'selected_mode':   mode,
        'date_from':       date_from,
        'date_to':         date_to,
    })

"""
def is_live():
    now_ts = int(time.time())
"""