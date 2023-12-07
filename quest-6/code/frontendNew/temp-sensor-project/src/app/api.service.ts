import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders } from '@angular/common/http';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root',
})
export class ApiService {
  private apiUrl = 'http://localhost:3004'; // Update this with your actual API URL

  constructor(private http: HttpClient) {}

  private getHeaders(): HttpHeaders {
    return new HttpHeaders({
      'Content-Type': 'application/json',
    });
  }

  // Example method to call the login API
  login(labName: string, labPassword: string): Observable<any> {
    const url = `${this.apiUrl}/login`;
    const params = { labName, labPassword };

    return this.http.get(url, { params, headers: this.getHeaders() });
  }

  // Add methods for other API endpoints as needed
  getAllConfig(labApi: string): Observable<any> {
    const url = `${this.apiUrl}/GetAllConfig`;
    const params = { labApi };

    return this.http.get(url, { params, headers: this.getHeaders() });
  }

  getHomePageData(labApi: string): Observable<any> {
    const url = `${this.apiUrl}/GetHomePageData`;
    const params = { labApi };

    return this.http.get(url, { params, headers: this.getHeaders() });
  }

  editDeviceConfig(labApi: string, deviceConfig: any): Observable<any> {
    const url = `${this.apiUrl}/EditDeviceConfig?labApi=${labApi}`;
    
    return this.http.put(url, deviceConfig, { headers: this.getHeaders() });
  }

  removeDevice(labApi: string, deviceID: string): Observable<any> {
    const url = `${this.apiUrl}/RemoveDevice?labApi=${labApi}&deviceID=${deviceID}`;
    
    return this.http.delete(url, { headers: this.getHeaders() });
  }

  // Add methods for other API endpoints as needed

  // Example for alarm-related methods
  getAllAlarms(labApi: string): Observable<any> {
    const url = `${this.apiUrl}/GetAllAlarms`;
    const params = { labApi };

    return this.http.get(url, { params, headers: this.getHeaders() });
  }

  addAlarm(labApi: string, alarmObject: any): Observable<any> {
    const url = `${this.apiUrl}/AddAlarm?labApi=${labApi}`;
    
    return this.http.post(url, alarmObject, { headers: this.getHeaders() });
  }

  editAlarm(labApi: string, updatedAlarm: any): Observable<any> {
    const url = `${this.apiUrl}/EditAlarm?labApi=${labApi}`;
    
    return this.http.put(url, updatedAlarm, { headers: this.getHeaders() });
  }

  removeAlarm(labApi: string, alarmID: string): Observable<any> {
    const url = `${this.apiUrl}/RemoveAlarm?labApi=${labApi}&alarmID=${alarmID}`;
    
    return this.http.delete(url, { headers: this.getHeaders() });
  }
}
