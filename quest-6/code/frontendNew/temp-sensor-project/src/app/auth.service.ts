// auth.service.ts

import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Injectable({
  providedIn: 'root',
})
export class AuthService {
  // Use localStorage to store authentication status
  get isAuthenticated(): boolean {
    return localStorage.getItem('isAuthenticated') === 'true';
  }

  set isAuthenticated(status: boolean) {
    localStorage.setItem('isAuthenticated', status.toString());
  }

  // Use localStorage to store labApi
  get labApi(): string {
    return localStorage.getItem('labApi') || '';
  }

  set labApi(api: string) {
    localStorage.setItem('labApi', api);
  }

  constructor(private http: HttpClient) {}

  // Check if the user is authenticated
  isAuthenticatedUser(): boolean {
    return this.isAuthenticated;
  }

  // API call for user login
  login(username: string, password: string): Promise<boolean> {
    const loginUrl = 'http://localhost:3004/login'; // Adjust the URL based on your API endpoint

    // Make an API call to authenticate the user
    return this.http
      .get(loginUrl, {
        params: {
          labName: username,
          labPassword: password,
        },
      })
      .toPromise()
      .then((response: any) => {
        if (response.success) {
          // Set isAuthenticated to true upon successful login
          this.isAuthenticated = true;
          this.labApi = response.api;
          return true;
        } else {
          // Clear isAuthenticated on failed login
          this.isAuthenticated = false;
          return false;
        }
      })
      .catch((error) => {
        console.error('Error during login:', error);
        // Clear isAuthenticated on error
        this.isAuthenticated = false;
        return false;
      });
  }

  // Simulate user logout
  logout(): void {
    // Clear authentication status on logout
    this.isAuthenticated = false;
  }
}
