import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-delete-device-modal',
  templateUrl: './delete-device-modal.component.html',
  styleUrls: ['./delete-device-modal.component.css']
})
export class DeleteDeviceModalComponent {
  @Input() deletedDevice: any;
  @Output() deleteDeviceEvent = new EventEmitter<void>();
  @Output() closeModalEvent = new EventEmitter<void>();

  deleteSensor(): void {
    // Implement your logic to save changes to the sensor
    this.deleteDeviceEvent.emit();
  }

  closeModal(): void {
    // Implement your logic to close the modal
    this.closeModalEvent.emit();
  }
}
