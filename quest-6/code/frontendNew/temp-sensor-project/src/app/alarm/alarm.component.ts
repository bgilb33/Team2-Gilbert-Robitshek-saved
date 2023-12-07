import { Component } from '@angular/core';
import { NgbModal, NgbModalRef } from '@ng-bootstrap/ng-bootstrap';
import { EditAlarmModalComponent } from '../edit-alarm-modal/edit-alarm-modal.component';
import { DeleteSensorModalComponent } from '../delete-sensor-modal/delete-sensor-modal.component';
import { AddAlarmModalComponent } from '../add-alarm-modal/add-alarm-modal.component';
import { ApiService } from '../api.service';
import { AuthService } from '../auth.service';

@Component({
  selector: 'app-alarm',
  templateUrl: './alarm.component.html',
  styleUrls: ['./alarm.component.css']
})
export class AlarmComponent {
  labApi: any;
  alarms: any[] = [];
  editedAlarm: any;
  deletedAlarm: any;
  isModalOpen = false;
  modalRef: NgbModalRef | null = null;

  constructor(
    private modalService: NgbModal,
    private apiService: ApiService,
    private authService: AuthService
  ) {}

  ngOnInit(): void {
    // Fetch labApi from AuthService
    this.labApi = this.authService.labApi;

    // Fetch data using labApi
    this.apiService.getAllAlarms(this.labApi).subscribe(
      (alarmData) => {
        this.alarms = alarmData;
      },
      (error) => {
        console.error('Error fetching config data:', error);
      }
    );
  }

  openEditModal(sensor: any): void {
    this.editedAlarm = { ...sensor };
    this.modalRef = this.modalService.open(EditAlarmModalComponent, { centered: true, size: 'lg' });
    this.modalRef.componentInstance.editedAlarm = this.editedAlarm;

    // Subscribe to the close and saveChanges events
    this.modalRef.componentInstance.closeModalEvent.subscribe(() => this.closeModal());
    this.modalRef.componentInstance.saveChangesEvent.subscribe((formData: any) => this.saveChanges(formData));

    this.isModalOpen = true;
  }

  openDeleteModal(sensor: any): void {
    this.deletedAlarm = { ...sensor };
    this.modalRef = this.modalService.open(DeleteSensorModalComponent, { centered: true, size: 'lg' });
    this.modalRef.componentInstance.deletedSensor = this.deletedAlarm;

    // Subscribe to the close and deleteSensor events
    this.modalRef.componentInstance.closeModalEvent.subscribe(() => this.closeModal());
    this.modalRef.componentInstance.deleteSensorEvent.subscribe(() => this.deleteSensor());

    this.isModalOpen = true;
  }

  openAlarmModal(): void {
    this.modalRef = this.modalService.open(AddAlarmModalComponent, { centered: true, size: 'lg' });
    
    this.modalRef.componentInstance.addAlarmEvent.subscribe((formData:any) => this.addAlarm(formData));
    this.modalRef.componentInstance.closeModalEvent.subscribe(() => this.closeModal());
    this.isModalOpen = true;
  }



  closeModal(): void {
    if (this.modalRef) {
      this.modalRef.close();
      this.isModalOpen = false;
    }
  }

  saveChanges(formData: any): void {
    console.log(formData);
    this.apiService.editAlarm(this.labApi, formData).subscribe(
      (response) => {
        if (response.success) {
          const index = this.alarms.findIndex(alarm => alarm.AlarmID === this.editedAlarm.AlarmID);

          if (index !== -1) {
            this.alarms[index] = { ...formData };
          }
        }
      },
      (error) => {
        console.error('Error updating alarm data:', error);
      }
    )

    this.closeModal();
  }

  deleteSensor(): void {
    const index = this.alarms.findIndex(alarm => alarm.AlarmID === this.deletedAlarm.AlarmID);

    if (index !== -1) {
      this.alarms.splice(index, 1);
    }

    console.log(this.alarms);

    this.closeModal();
  }

  addAlarm(formData:any): void {

    //NEED TO FIGURE THIS OUT. LOOK AT API??

    console.log(formData);

    const index = this.alarms.length;
    formData.AlarmID = index;
    this.alarms.push(formData);
    this.closeModal();

  }

  editAlarm(alarm: any): void {
    this.openEditModal(alarm);
  }

  deleteAlarm(alarm: any): void {
    this.openDeleteModal(alarm);
  }
}
