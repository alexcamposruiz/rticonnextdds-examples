# (c) Copyright, Real-Time Innovations, 2022.  All rights reserved.
# RTI grants Licensee a license to use, modify, compile, and create derivative
# works of the software solely for use with RTI Connext DDS. Licensee may
# redistribute copies of the software provided that all such copies are subject
# to this license. The software is provided "as is", with no warranty of any
# type, including any warranty for fitness for any purpose. RTI is under no
# obligation to maintain or support the software. RTI shall not be liable for
# any incidental or consequential damages arising out of the use or inability
# to use the software.

import sys
import asyncio

import rti.connextdds as dds
import rti.asyncio

from partitions import PartitionsExample


class PartitionsExamplePublisher:
    def __init__(self, domain_id: int):
        self.participant = dds.DomainParticipant(domain_id)

        topic = dds.Topic(
            self.participant, "Example partitions", PartitionsExample
        )

        self.publisher = dds.Publisher(self.participant)

        self.writer = dds.DataWriter(self.publisher, topic)

        self.samples_written = 0

    async def run(self, sample_count: int):
        sample = PartitionsExample()
        while self.samples_written < sample_count:
            # Modify the data to be sent here
            sample.x = self.samples_written

            # Every 5 samples we will change the partition name.
            new_partitions = None
            if (self.samples_written + 1) % 15 == 0:
                # Multiple partitions, with match
                new_partitions = ["USA/CA/Sunnyvale", "USA/CA/San Francisco"]
            elif (self.samples_written + 1) % 15 == 5:
                # Wildcard match
                new_partitions = ["USA/CA/*"]
            elif (self.samples_written + 1) % 15 == 10:
                # No match.
                new_partitions = ["USA/NV/Las Vegas"]

            if new_partitions is not None:
                publisher_qos = self.publisher.qos
                publisher_qos.partition.name = new_partitions
                self.publisher.qos = publisher_qos
                print(
                    f"Partition set to: {list(self.publisher.qos.partition.name)}"
                )

            print(f"Writing {sample}")
            self.writer.write(sample)

            self.samples_written += 1
            await asyncio.sleep(1)


if __name__ == "__main__":
    try:
        rti.asyncio.run(PartitionsExamplePublisher(0).run(sys.maxsize))
    except KeyboardInterrupt:
        pass
