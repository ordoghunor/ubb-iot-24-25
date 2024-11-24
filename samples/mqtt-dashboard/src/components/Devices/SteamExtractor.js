import { useState, useRef } from "react";
import mqtt from "mqtt";
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import Form from 'react-bootstrap/Form';
import Button from "react-bootstrap/Button";
import '../../styles/CustomSwitch.css';

export default function SteamExtractor({ topic, location }) {
    const [isConnected, setIsConnected] = useState(false);
    const [isChecked, setIsChecked] = useState(false);

    const clientRef = useRef(null);

    const handleSwitchConnection = () => {
        if (!isConnected) {
            const willMessage = JSON.stringify({
                device: `${location} smoke sensor`,
                status: "Unexpected disconnected",
            });

            clientRef.current = mqtt.connect('mqtt://localhost:9001', {
                will: {
                    topic: `home/problems`,
                    payload: willMessage,
                    qos: 1,
                    retain: false,
                }
            });
            clientRef.current.on('connect', function () {
                setIsConnected(true);
                console.log(`connected ${location} steam extractor`);
            });

            clientRef.current.subscribe(topic, function () {
                console.log(`${location} steam extractor subscribed to ${topic}'`);
            });

            clientRef.current.on('message', function (receivedTopic, mesage) {
                if (receivedTopic === topic) {
                    console.log(`${location} steam extractor received: ${mesage} on ${receivedTopic}`);
                    const messObject = JSON.parse(mesage);
                    setIsChecked(prevChecked => {
                        if (prevChecked && Number(messObject.value) <= 10)
                            return false;
                        if (!prevChecked && Number(messObject.value) >= 60)
                            return true;
                        return prevChecked;
                    });
                }
            });
        } else {
            if (clientRef.current) {
                clientRef.current.unsubscribe(topic, function () {
                    console.log(`${location} steam extractor: unsubscribed from topic: ${topic}`);
                });
                console.log(`deconnected ${location} steam extractor`);
                clientRef.current.end(); // Close the MQTT connection
                clientRef.current = null;
            }
            setIsConnected(false);
        }
    }

    const simulateUnexpectedDisconnect = () => {
        console.log(clientRef.current);
        if (isConnected) {
            console.log(`Simulating unexpected disconnect for ${location} steam extractor`);
            clientRef.current.stream.end();
            // clientRef.current = null;
            setIsConnected(false);
        }
    }

    return (
        <>
            <Row>
                <h2>Steam Extractor</h2>
            </Row>
            <Row>
                <div className="form-check-container">
                    <h4>Connection status:</h4>
                    <Form.Check
                        type="switch"
                        id="custom-switch"
                        checked={isConnected}
                        className="checker"
                        onChange={handleSwitchConnection}
                    />
                </div>
            </Row>
            <Row>
                <h4>Location: {location}</h4>
            </Row>
            <Row className='button_row p-1'>
                <Col className='button_col'>
                    Is running:
                </Col>
                <Col className='button_col'>
                    <Form.Check
                        type="switch"
                        id="custom-switch"
                        checked={isChecked}
                        readOnly
                        className="checker"
                    />
                </Col>
            </Row>
            <Row className='p-2'>
                <Button onClick={simulateUnexpectedDisconnect} variant="danger">
                    Simulate Unexpected Disconnect
                </Button>
            </Row>
        </>
    );
}
