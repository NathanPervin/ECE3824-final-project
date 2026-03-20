from django.shortcuts import render
from django.contrib.auth.decorators import login_required
from api.models import CO2Reading

# Create your views here.
@login_required(login_url='/users/login_user')  # redirects here if not logged in
def co2_table(request):
    readings = CO2Reading.objects.order_by("-unix_timestamp")[:100]
    return render(request, "dashboard/co2_table.html", {"readings": readings})