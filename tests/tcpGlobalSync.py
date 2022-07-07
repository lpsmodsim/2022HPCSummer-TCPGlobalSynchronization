import sst

router = sst.Component("Router", "tcpGlobalSync.router")
router.addParams(
    {
        "tickFreq": "2s",
        "numPorts": "1",
        "verbose_level": "3",
        "queueSize": "3",
    }
)

client_zero = sst.Component("Client_0", "tcpGlobalSync.client")
client_zero.addParams(
    {
        "tickFreq": "1s",
        "timeout": "25",
        "node_id": "0",
        "verbose_level": "4",
    }
)


"""client_one = sst.Component("Client_1", "tcpGlobalSync.client")
client_one.addParams(
    {
        "tickFreq": "1s",
        "timeout": "25",
        "node_id": "1",
        "verbose_level": "4",
    }
)"""

sst.Link("Flow_One").connect(
    (client_zero, "commPort", "1s"), (router, "commPort0", "1s")
)

"""sst.Link("Flow_Two").connect(
    (client_one, "commPort", "1s"), (router, "commPort1", "1s")
)"""
