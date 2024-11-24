import { useState, useRef } from "react";
import mqtt from "mqtt";
import Row from 'react-bootstrap/Row';
import Form from 'react-bootstrap/Form';
import '../../styles/CustomSwitch.css';

export default function ProblemDetector() {
    const [isConnected, setIsConnected] = useState(false);
    const [problems, setProblems] = useState([]);

    const topic = 'home/problems'
    const clientRef = useRef(null);

    const handleSwitchConnection = () => {
        if(!isConnected) 
        {
            clientRef.current = mqtt.connect('mqtt://localhost:9001', { clean: false, clientId: 'problem_detector' });
            clientRef.current.on('connect', function () {
                setIsConnected(true);
                console.log(`connected problem detector`);
            });

            clientRef.current.subscribe(topic,{ qos: 1 }, function () {
                console.log(`problem detector subscribed to ${topic}'`);
           });
   
           clientRef.current.on('message', function (receivedTopic, mess) {
               if (receivedTopic === topic) {
                   console.log(`Problem detector received: ${mess} on ${receivedTopic}`);
                   const messObject = JSON.parse(mess);
                   setProblems(prevProblems => [...prevProblems, messObject]);
               }
           });
        } else {
            if(clientRef.current) {
                // clientRef.current.unsubscribe(topic, function () {
                //     console.log(`problem detector: unsubscribed from topic: ${topic}`);
                // });
                console.log(`deconnected problem detector`);
                clientRef.current.end(); // Close the MQTT connection
                clientRef.current = null;
            }
            setIsConnected(false);
        }
    }

    return (
        <>
            <Row>
                <h2>Problem detector</h2>
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
            {problems.map((problem) => (
            <Row>
                <p>{problem.device} - {problem.status}</p>
            </Row>
            ))}
        </>
    );
}
