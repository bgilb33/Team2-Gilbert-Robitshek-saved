import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DeleteSensorModalComponent } from './delete-sensor-modal.component';

describe('DeleteSensorModalComponent', () => {
  let component: DeleteSensorModalComponent;
  let fixture: ComponentFixture<DeleteSensorModalComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ DeleteSensorModalComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(DeleteSensorModalComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
