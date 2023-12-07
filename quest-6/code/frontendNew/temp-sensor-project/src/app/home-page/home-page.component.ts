import { Component } from '@angular/core';
import { ApiService } from '../api.service';
import { AuthService } from '../auth.service';

@Component({
  selector: 'app-home-page',
  templateUrl: './home-page.component.html',
  styleUrls: ['./home-page.component.css']
})
export class HomePageComponent {
  labApi: any;
  // Need to implement way to get this. Probably get from authService or new endpoint
  labName: any = "NIA Lab";
  tempSensors: any[] = [];

  ngOnInit(): void {
    // Fetch labApi from AuthService
    this.labApi = this.authService.labApi;

    // Fetch data using labApi
    this.apiService.getHomePageData(this.labApi).subscribe(
      (homePageData) => {
        this.tempSensors = homePageData.data; // Adjust the property based on your API response
      },
      (error) => {
        console.error('Error fetching config data:', error);
      }
    );
  }

  constructor(
    private apiService: ApiService,
    private authService: AuthService
  ) {}

  convertEpochToDateTime(epochTime: number): string {
    // Convert epoch time to milliseconds
    const date = new Date(epochTime * 1000);
    return date.toLocaleString('en-US', { timeZone: "America/New_York" });
  }
}
