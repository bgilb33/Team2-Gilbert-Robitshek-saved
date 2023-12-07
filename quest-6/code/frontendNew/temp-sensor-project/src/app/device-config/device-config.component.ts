import { Component } from '@angular/core';
import { NgbModal, NgbModalRef } from '@ng-bootstrap/ng-bootstrap';
import { EditSensorModalComponent } from '../edit-sensor-modal/edit-sensor-modal.component';
import { DeleteSensorModalComponent } from '../delete-sensor-modal/delete-sensor-modal.component';
import { ApiService } from '../api.service';
import { AuthService } from '../auth.service';

@Component({
  selector: 'app-device-config',
  templateUrl: './device-config.component.html',
  styleUrls: ['./device-config.component.css'],
})
export class DeviceConfigComponent {
  tempSensors: any[] = [];
  editedSensor: any;
  deletedSensor: any;
  isModalOpen = false;
  modalRef: NgbModalRef | null = null;
  labApi: string = ''; 

  constructor(
    private modalService: NgbModal,
    private apiService: ApiService,
    private authService: AuthService
  ) {}

  ngOnInit(): void {
    // Fetch labApi from AuthService
    this.labApi = this.authService.labApi;

    // Fetch data using labApi
    this.apiService.getAllConfig(this.labApi).subscribe(
      (configData) => {
        this.tempSensors = configData.configData; // Adjust the property based on your API response
      },
      (error) => {
        console.error('Error fetching config data:', error);
      }
    );
  }

  openEditModal(sensor: any): void {
    this.editedSensor = { ...sensor };
    this.modalRef = this.modalService.open(EditSensorModalComponent, { centered: true, size: 'lg' });
    this.modalRef.componentInstance.editedSensor = this.editedSensor;

    // Subscribe to the close and saveChanges events
    this.modalRef.componentInstance.closeModalEvent.subscribe(() => this.closeModal());
    this.modalRef.componentInstance.saveChangesEvent.subscribe((formData: any) => this.saveChanges(formData));

    this.isModalOpen = true;
  }

  openDeleteModal(sensor: any): void {
    this.deletedSensor = { ...sensor };
    this.modalRef = this.modalService.open(DeleteSensorModalComponent, { centered: true, size: 'lg' });
    this.modalRef.componentInstance.deletedSensor = this.deletedSensor;

    // Subscribe to the deleteSensor and closeModal events
    this.modalRef.componentInstance.deleteSensorEvent.subscribe(() => this.deleteSensor());
    this.modalRef.componentInstance.closeModalEvent.subscribe(() => this.closeModal());

    this.isModalOpen = true;
  }

  closeModal(): void {
    if (this.modalRef) {
      this.modalRef.close();
      this.isModalOpen = false;
    }
  }

  // EDIT W API
  saveChanges(formData: any): void {

    this.apiService.editDeviceConfig(this.labApi, formData).subscribe(
      (response) => {
        if (response.success) {
          const index = this.tempSensors.findIndex(sensor => sensor.DeviceID === this.editedSensor.DeviceID);
          console.log("INDEX: ", index);
          if (index !== -1) {
            this.tempSensors[index] = { ...formData };
          }
        }
      },
      (error) => {
        console.error('Error updating config data:', error);
      }
    )


    this.closeModal();
  }

  // DELETE W API
  deleteSensor(): void {
    // NEED TO IMPLEMENT W API
    // Close the modal after deleting the sensor
    const index = this.tempSensors.findIndex(sensor => sensor.DeviceID === this.deletedSensor.DeviceID);

    if (index !== -1) {
      this.tempSensors.splice(index, 1);
    }

    console.log(this.tempSensors);
    this.closeModal();
  }

  // Opens edit modal
  editDevice(sensor: any): void {
    this.openEditModal(sensor);
  }

  // Opens delete modal
  deleteDevice(sensor: any): void {
    this.openDeleteModal(sensor);
  }
}
