// edit-sensor-modal.component.ts

import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-delete-sensor-modal',
  templateUrl: './delete-sensor-modal.component.html',
  styleUrls: ['./delete-sensor-modal.component.css'],
})
export class DeleteSensorModalComponent {
  @Input() deletedSensor: any;
  @Output() deleteSensorEvent = new EventEmitter<void>();
  @Output() closeModalEvent = new EventEmitter<void>();

  deleteSensor(): void {
    // Implement your logic to save changes to the sensor
    this.deleteSensorEvent.emit();
  }

  closeModal(): void {
    // Implement your logic to close the modal
    this.closeModalEvent.emit();
  }
}
