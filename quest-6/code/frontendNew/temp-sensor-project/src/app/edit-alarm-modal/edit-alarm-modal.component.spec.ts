import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditAlarmModalComponent } from './edit-alarm-modal.component';

describe('EditAlarmModalComponent', () => {
  let component: EditAlarmModalComponent;
  let fixture: ComponentFixture<EditAlarmModalComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ EditAlarmModalComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(EditAlarmModalComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
