import { ComponentFixture, TestBed } from '@angular/core/testing';

import { AddAlarmModalComponent } from './add-alarm-modal.component';

describe('AddAlarmModalComponent', () => {
  let component: AddAlarmModalComponent;
  let fixture: ComponentFixture<AddAlarmModalComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ AddAlarmModalComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(AddAlarmModalComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
