import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-add-alarm-modal',
  templateUrl: './add-alarm-modal.component.html',
  styleUrls: ['./add-alarm-modal.component.css']
})
export class AddAlarmModalComponent {
  @Output() addAlarmEvent = new EventEmitter<any>();
  @Output() closeModalEvent = new EventEmitter<void>();

  newAlarm = {
    AlarmID: -1,
    DeviceID: -1,
    DeviceName: "",
    SensorType: "",
    Compare: "",
    Threshold: "",
    Status: "Deactivated"
  }
  
  newAlarmCopy: any;

  ngOnInit() {
    // Initialize the copy in the ngOnInit lifecycle hook
    this.newAlarmCopy = { ...this.newAlarm };
  }

  addAlarm(): void {
    // Update the editedAlarmCopy with the form data
    this.newAlarmCopy = { ...this.newAlarm };

    // Emit the form data
    this.addAlarmEvent.emit(this.newAlarmCopy);
  }

  closeModal(): void {
    // Implement your logic to close the modal
    this.closeModalEvent.emit();
  }

}
