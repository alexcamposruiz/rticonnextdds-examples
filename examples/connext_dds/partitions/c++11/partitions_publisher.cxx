/*******************************************************************************
 (c) 2005-2015 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 RTI grants Licensee a license to use, modify, compile, and create derivative
 works of the Software.  Licensee has the right to distribute object form only
 for use with RTI products.  The Software is provided "as is", with no warranty
 of any type, including any warranty for fitness for any purpose. RTI is under
 no obligation to maintain or support the Software.  RTI shall not be liable for
 any incidental or consequential damages arising out of the use or inability to
 use the software.
 ******************************************************************************/

#include <dds/pub/ddspub.hpp>
#include <rti/util/util.hpp>      // for sleep()
#include <rti/config/Logger.hpp>  // for logging

#include "application.hpp"  // for command line parsing and ctrl-c
#include "partitions.hpp"

void run_publisher_application(
        unsigned int domain_id,
        unsigned int sample_count)
{
    // Create a DomainParticipant with default Qos.
    dds::domain::DomainParticipant participant(domain_id);

    // Retrieve the default Publisher QoS, from USER_QOS_PROFILES.xml
    dds::pub::qos::PublisherQos publisher_qos =
            dds::core::QosProvider::Default().publisher_qos();
    auto &partition = publisher_qos.policy<dds::core::policy::Partition>();
    std::vector<std::string> partition_names = partition.name();
    std::vector<std::string> sensor_ids {"sensor1", "sensor2", "sensor3"};
    // If you want to change the Publisher QoS programmatically rather
    // than using the XML file, you will need to comment out these lines.

    // partition_names[0] = "USA/CA/Sunnyvale";
    // partition_names[1] = "USA/NV/*";
    // partition.name(partition_names);
    // publisher_qos << partition;

    std::cout << "Setting partition to";
    for (const auto &name : partition_names) {
        std::cout << " '" << name << "'";
    }
    std::cout << std::endl;

    // Create a Publisher.
    dds::pub::Publisher publisher(participant, publisher_qos);

    // Create a Topic -- and automatically register the type.
    dds::topic::Topic<Temperature> topic(participant, "Example partitions");

    // Retrieve the default DataWriter QoS, from USER_QOS_PROFILES.xml
    dds::pub::qos::DataWriterQos writer_qos =
            dds::core::QosProvider::Default().datawriter_qos();

    // If you want to change the DataWriter QoS programmatically rather
    // than using the XML file, you will need to comment out these lines.

    // writer_qos << Reliability::Reliable()
    //            << History::KeepLast(3)
    //            << Durability::TransientLocal();

    // Create a Datawriter.
    dds::pub::DataWriter<Temperature> writer(publisher, topic, writer_qos);

    // Create a data sample for writing.
    Temperature instance;

    // Main loop
    bool update_qos = false;
    for (unsigned int samples_written = 0;
         !application::shutdown_requested && samples_written < sample_count;
         samples_written++) {
        std::cout << "Writing Temperature, count " << samples_written
                  << std::endl;

        // Modify and send the sample.
        instance.sensor_id(sensor_ids[samples_written % sensor_ids.size()]);
        instance.value(samples_written);
        writer.write(instance);

        // Every 5 samples we will change the partition name.
        if ((samples_written + 1) % 15 == 0) {
            // Multiple partitions, with match
            partition_names.resize(2);
            partition_names[0] = "USA/CA/Sunnyvale";
            partition_names[1] = "USA/CA/San Francisco";
            update_qos = true;
        } else if ((samples_written + 1) % 15 == 5) {
            // Wildcard match
            partition_names.resize(1);
            partition_names[0] = "USA/CA/*";
            update_qos = true;
        } else if ((samples_written + 1) % 15 == 10) {
            // No match
            partition_names.resize(1);
            partition_names[0] = "USA/NV/Las Vegas";
            update_qos = true;
        }

        // Set the new partition names to the publisher QoS.
        if (update_qos) {
            std::cout << "Setting partition to";
            for (int i = 0; i < partition_names.size(); i++) {
                std::cout << " '" << partition_names[i] << "'";
            }
            std::cout << std::endl;

            partition.name(partition_names);
            publisher.qos(publisher_qos << partition);
            update_qos = false;
        }

        rti::util::sleep(dds::core::Duration(1));
    }
}

int main(int argc, char *argv[])
{
    using namespace application;

    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::exit) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::failure) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_publisher_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception &ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_publisher_application(): " << ex.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
