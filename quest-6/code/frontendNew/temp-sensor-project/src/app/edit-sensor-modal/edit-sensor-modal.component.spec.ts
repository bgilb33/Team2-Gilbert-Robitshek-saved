import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditSensorModalComponent } from './edit-sensor-modal.component';

describe('EditSensorModalComponent', () => {
  let component: EditSensorModalComponent;
  let fixture: ComponentFixture<EditSensorModalComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ EditSensorModalComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(EditSensorModalComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
