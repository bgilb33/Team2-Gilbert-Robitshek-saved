// edit-alarm-modal.component.ts

import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-edit-alarm-modal',
  templateUrl: './edit-alarm-modal.component.html',
  styleUrls: ['./edit-alarm-modal.component.css']
})
export class EditAlarmModalComponent {
  @Input() editedAlarm: any;
  @Output() saveChangesEvent = new EventEmitter<any>();
  @Output() closeModalEvent = new EventEmitter<void>();

  // Create a copy of the editedAlarm object
  editedAlarmCopy: any;

  ngOnInit() {
    // Initialize the copy in the ngOnInit lifecycle hook
    this.editedAlarmCopy = { ...this.editedAlarm };
  }

  saveChanges(): void {
    // Update the editedAlarmCopy with the form data
    this.editedAlarmCopy = { ...this.editedAlarm };

    // Emit the form data
    this.saveChangesEvent.emit(this.editedAlarmCopy);
  }

  closeModal(): void {
    // Implement your logic to close the modal
    this.closeModalEvent.emit();
  }
}
