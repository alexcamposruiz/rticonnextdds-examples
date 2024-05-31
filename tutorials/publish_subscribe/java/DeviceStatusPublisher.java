/*
* (c) Copyright, Real-Time Innovations, 2024.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

import java.util.Objects;
import java.lang.reflect.*;

import com.rti.dds.domain.DomainParticipant;
import com.rti.dds.domain.DomainParticipantFactory;
import com.rti.dds.infrastructure.InstanceHandle_t;
import com.rti.dds.infrastructure.StatusKind;
import com.rti.dds.publication.Publisher;
import com.rti.dds.topic.Topic;


public class DeviceStatusPublisher extends Application implements AutoCloseable {

    private DomainParticipant participant = null;

    private void runApplication(String sensorName, String roomName) {
        participant = Objects.requireNonNull(
            DomainParticipantFactory.get_instance().create_participant(
                0,
                DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT,
                null, // listener
                StatusKind.STATUS_MASK_NONE));

        Publisher publisher = Objects.requireNonNull(
            participant.create_publisher(
                DomainParticipant.PUBLISHER_QOS_DEFAULT,
                null, // listener
                StatusKind.STATUS_MASK_NONE));

        String typeName = DeviceStatusTypeSupport.get_type_name();
        DeviceStatusTypeSupport.register_type(participant, typeName);

        Topic topic = Objects.requireNonNull(
            participant.create_topic(
                "WindowStatus",
                typeName,
                DomainParticipant.TOPIC_QOS_DEFAULT,
                null, // listener
                StatusKind.STATUS_MASK_NONE));

        DeviceStatusDataWriter writer = (DeviceStatusDataWriter) Objects.requireNonNull(
            publisher.create_datawriter(
                topic,
                Publisher.DATAWRITER_QOS_DEFAULT,
                null, // listener
                StatusKind.STATUS_MASK_NONE));

        DeviceStatus deviceStatus = new DeviceStatus();
        deviceStatus.sensor_name = sensorName;
        deviceStatus.room_name = roomName;

        for (int samplesWritten = 0;
                !isShutdownRequested() && samplesWritten < 1000;
                samplesWritten++) {

            deviceStatus.is_open = !deviceStatus.is_open;
            writer.write(deviceStatus, InstanceHandle_t.HANDLE_NIL);
            try {
                final long sendPeriodMillis = 2000; // 1 second
                Thread.sleep(sendPeriodMillis);
            } catch (InterruptedException ix) {
                break;
            }
        }
    }

    @Override
    public void close() {
        if (participant != null) {
            participant.delete_contained_entities();

            DomainParticipantFactory.get_instance()
            .delete_participant(participant);
        }
    }


    public static void main(String[] args) {

        String sensorName = args.length > 0 ? args[0] : "Sensor1";
        String roomName = args.length > 1 ? args[1] : "LivingRoom";

        try (DeviceStatusPublisher publisherApplication = new DeviceStatusPublisher()) {
            publisherApplication.addShutdownHook();
            publisherApplication.runApplication(sensorName, roomName);
        }

        DomainParticipantFactory.finalize_instance();
    }
}
