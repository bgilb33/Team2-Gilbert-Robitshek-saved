import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DeleteDeviceModalComponent } from './delete-device-modal.component';

describe('DeleteDeviceModalComponent', () => {
  let component: DeleteDeviceModalComponent;
  let fixture: ComponentFixture<DeleteDeviceModalComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ DeleteDeviceModalComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(DeleteDeviceModalComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
