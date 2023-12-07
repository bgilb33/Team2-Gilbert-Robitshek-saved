import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NavBarComponent } from '../nav-bar/nav-bar.component';

@Component({
  selector: 'app-home-page',
  standalone: true,
  imports: [CommonModule, NavBarComponent],
  templateUrl: './home-page.component.html',
  styleUrl: './home-page.component.css'
})
export class HomePageComponent {
  labName = "Benji's Lab"
  activeDevices = 3;
  tempSensors = [
    {
      Name: "Lab Room 1",
      Temperature: "89.0\u00B0C",
      Humidity: "67.2%"
    },
    {
      Name: "Fridge 1",
      Temperature: "34.0\u00B0C",
      Humidity: "15.2%"
    },
    {
      Name: "Conference Room",
      Temperature: "73.0\u00B0C",
      Humidity: ".267%"
    },
    {
      Name: "Lab Room 2",
      Temperature: "89.0\u00B0C",
      Humidity: "67.2%"
    },
    {
      Name: "Fridge 2",
      Temperature: "34.0\u00B0C",
      Humidity: "15.2%"
    },
    {
      Name: "Snack Room",
      Temperature: "73.0\u00B0C",
      Humidity: ".267%"
    }
]
}
