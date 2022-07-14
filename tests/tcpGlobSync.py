import sst

sender_zero = sst.Component("Sender Node Zero", "tcpGlobSync.sender")
sender_zero.addParams(
    {
        "tickFreq": "1s",
        "low_send_rate": "5",
        "max_send_rate": "999",
        "verbose_level": "1",
        "node_id": "0",
        "starting_cycle": "1",
    }
)

sender_one = sst.Component("Sender Node One", "tcpGlobSync.sender")
sender_one.addParams(
    {
        "tickFreq": "1s",
        "low_send_rate": "5",
        "max_send_rate": "999",
        "verbose_level": "1",
        "node_id": "1",
        "starting_cycle": "10",
    }
)

sender_two = sst.Component("Sender Node Two", "tcpGlobSync.sender")
sender_two.addParams(
    {
        "tickFreq": "1s",
        "low_send_rate": "5",
        "max_send_rate": "999",
        "verbose_level": "1",
        "node_id": "2",
        "starting_cycle": "25",
    }
)

receiver = sst.Component("Receiver Node", "tcpGlobSync.receiver")
receiver.addParams(
    {
        "tickFreq": "1s",
        "process_rate": "250",
        "queue_size": "1000",
        "verbose_level": "1",
        "num_nodes": "3",
        "window_size": "100",
    }
)

sst.Link("Link_Zero").connect((sender_zero, "port", "1ms"), (receiver, "port0", "1ms"))
sst.Link("Link_One").connect((sender_one, "port", "1ms"), (receiver, "port1", "1ms"))
sst.Link("Link_Two").connect((sender_two, "port", "1ms"), (receiver, "port2", "1ms"))
