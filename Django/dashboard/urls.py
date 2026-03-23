from django.urls import path
from . import views

urlpatterns = [
    path("table/", views.co2_table, name="co2_table"),
    path('search/', views.search, name='search'),
    path('sessions/<str:building>/<str:room>/', views.sessions, name='sessions'),
]