/*
 * (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 *  RTI grants Licensee a license to use, modify, compile, and create derivative
 *  works of the Software.  Licensee has the right to distribute object form
 *  only for use with RTI products.  The Software is provided "as is", with no
 *  warranty of any type, including any warranty for fitness for any purpose.
 *  RTI is under no obligation to maintain or support the Software.  RTI shall
 *  not be liable for any incidental or consequential damages arising out of the
 *  use or inability to use the software.
 */

#include "ndds/ndds_c.h"
#include "home_automation.h"
#include "home_automationSupport.h"

static int subscriber_shutdown(
    DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
            DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    return status;
}

void DeviceStatusListener_on_data_available(
    void* listener_data,
    DDS_DataReader* reader)
{
    DeviceStatusDataReader *DeviceStatus_reader = NULL;
    struct DeviceStatusSeq data_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq info_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_Time_t timestamp;
    DeviceStatus *device_status = NULL;
    DDS_ReturnCode_t retcode;
    int i;


    DeviceStatus_reader = DeviceStatusDataReader_narrow(reader);
    if (DeviceStatus_reader == NULL) {
        fprintf(stderr, "DataReader narrow error\n");
        return;
    }

    retcode = DeviceStatusDataReader_take(
            DeviceStatus_reader,
            &data_seq,
            &info_seq,
            DDS_LENGTH_UNLIMITED,
            DDS_ANY_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE,
            DDS_ANY_INSTANCE_STATE);
    if (retcode == DDS_RETCODE_NO_DATA) {
        return;
    } else if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "take error %d\n", retcode);
        return;
    }

    for (i = 0; i < DeviceStatusSeq_get_length(&data_seq); ++i) {
        if (DDS_SampleInfoSeq_get_reference(&info_seq, i)->valid_data) {
            timestamp = DDS_SampleInfoSeq_get_reference(&info_seq, i)->source_timestamp;
            device_status = DeviceStatusSeq_get_reference(&data_seq, i);
            if (device_status->is_open) {
                printf("WARNING: %s in %s is open (%.3f s)\n",
                        device_status->sensor_name,
                        device_status->room_name,
                        timestamp.sec + timestamp.nanosec / 1e9);
            }
        }
    }

    retcode = DeviceStatusDataReader_return_loan(
            DeviceStatus_reader,
            &data_seq,
            &info_seq);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "return loan error %d\n", retcode);
    }
}

int monitor_sensor(void)
{

    DDS_DomainParticipant *participant = NULL;
    DDS_Subscriber *subscriber = NULL;
    DDS_Topic *topic = NULL;
    struct DDS_DataReaderListener reader_listener =
            DDS_DataReaderListener_INITIALIZER;
    DDS_DataReader *reader = NULL;
    const char *type_name = NULL;
    DDS_ReturnCode_t retcode;
    struct DDS_Duration_t poll_period = {2,0};
    int i;

    participant = DDS_DomainParticipantFactory_create_participant(
            DDS_TheParticipantFactory,
            0,
            &DDS_PARTICIPANT_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);

    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    subscriber = DDS_DomainParticipant_create_subscriber(
            participant,
            &DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);

    if (subscriber == NULL) {
        fprintf(stderr, "create_subscriber error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    type_name = DeviceStatusTypeSupport_get_type_name();
    retcode = DeviceStatusTypeSupport_register_type(
            participant,
            type_name);

    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        subscriber_shutdown(participant);
        return -1;
    }

    topic = DDS_DomainParticipant_create_topic(
            participant,
            "WindowStatus",
            type_name,
            &DDS_TOPIC_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);

    if (topic == NULL) {
        fprintf(stderr, "create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    reader_listener.on_data_available =
            DeviceStatusListener_on_data_available;

    reader = DDS_Subscriber_create_datareader(
            subscriber,
            DDS_Topic_as_topicdescription(topic),
            &DDS_DATAREADER_QOS_DEFAULT,
            &reader_listener,
            DDS_DATA_AVAILABLE_STATUS);
    if (reader == NULL) {
        fprintf(stderr, "create_datareader error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    for (i = 0; i < 1000; i++) {
        NDDS_Utility_sleep(&poll_period);
    }

    return subscriber_shutdown(participant);
}

int main(int argc, char **argv)
{
    return monitor_sensor();
}
