import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-edit-sensor-modal',
  templateUrl: './edit-sensor-modal.component.html',
  styleUrls: ['./edit-sensor-modal.component.css'],
})
export class EditSensorModalComponent {
  @Input() editedSensor: any;
  @Output() saveChangesEvent = new EventEmitter<any>();
  @Output() closeModalEvent = new EventEmitter<void>();

  editedSensorCopy: any;

  ngOnInit() {
    this.editedSensorCopy = { ...this.editedSensor };
  }

  saveChanges(): void {
    console.log(this.editedSensor);
    this.editedSensorCopy = { ...this.editedSensor };

    this.saveChangesEvent.emit(this.editedSensorCopy);
  }

  closeModal(): void {
    // Implement your logic to close the modal
    this.closeModalEvent.emit();
  }
}
