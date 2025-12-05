let map = L.map('map').setView([33.6844,73.0479],12);
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',{attribution:'Map data © OpenStreetMap contributors'}).addTo(map);

let branch=null,locations=[],riders=[],foods=[],orders=[];
let riderMarkers=[],orderMarkers=[];
let foodPrice=0,deliveryPrice=0,finalPrice=0;

// Load data.json
fetch('data.json').then(res=>res.json()).then(data=>{
    branch=data.branch;
    locations=data.locations;
    riders=data.riders;
    foods=data.foods;

    // Branch marker
    L.marker([branch.lat,branch.lng],{icon:L.icon({iconUrl:'https://cdn-icons-png.flaticon.com/512/684/684908.png',iconSize:[35,35]})}).addTo(map).bindPopup("Branch: "+branch.name).openPopup();

    // Locations dropdown
    locations.forEach(loc=>{
        let option=document.createElement('option');
        option.value=loc.name;
        option.textContent=loc.name+" (Delivery PKR "+loc.delivery+")";
        document.getElementById('locationSelect').appendChild(option);
    });

    // Riders markers
    riders.forEach((r,i)=>{
        let m=L.marker([r.lat,r.lng],{icon:L.icon({iconUrl:'https://cdn-icons-png.flaticon.com/512/194/194221.png',iconSize:[30,30]})}).addTo(map);
        m.bindPopup(r.name);
        riderMarkers.push(m);
    });

    updateRiderStatus();
    displayFoods(foods);
});

// Display Food List
function displayFoods(foodArray){
    let ul=document.getElementById('foodList');
    ul.innerHTML='';
    foodArray.forEach(f=>{
        let li=document.createElement('li');
        li.textContent=f.name+" (PKR "+f.price+")";
        li.addEventListener('click',()=>{
            orders.push(f);
            foodPrice+=f.price;
            updateOrderList();
        });
        ul.appendChild(li);
    });
}

// Search Food
document.getElementById('foodSearch').addEventListener('input',e=>{
    let search=e.target.value.toLowerCase();
    let filtered=foods.filter(f=>f.name.toLowerCase().includes(search));
    displayFoods(filtered);
});

// Update Order List
function updateOrderList(){
    let ul=document.getElementById('orderList');
    ul.innerHTML='';
    orders.forEach((o,i)=>{
        let li=document.createElement('li');
        li.textContent=`${i+1}. ${o.name} (PKR ${o.price})`;
        li.addEventListener('click',()=>{
            foodPrice-=o.price;
            orders.splice(i,1);
            updateOrderList();
        });
        ul.appendChild(li);
    });
    document.getElementById('foodPrice').textContent=foodPrice;
    calculateFinalPrice();
}

// Calculate Delivery + Final Price
function calculateFinalPrice(){
    let locName=document.getElementById('locationSelect').value;
    let loc=locations.find(l=>l.name===locName);
    deliveryPrice=loc?loc.delivery:0;
    finalPrice=foodPrice+deliveryPrice;
    document.getElementById('deliveryPrice').textContent=deliveryPrice;
    document.getElementById('finalPrice').textContent=finalPrice;
}

// Assign Nearest Rider & Deliver
document.getElementById('assignRider').addEventListener('click',()=>{
    if(orders.length===0){alert("Select some food!");return;}
    let locName=document.getElementById('locationSelect').value;
    let loc=locations.find(l=>l.name===locName);
    if(!loc){alert("Select delivery location");return;}

    // Find nearest available rider
    let nearest=null;
    let minDist=Infinity;
    riders.forEach((r,i)=>{
        if(r.status==='Available'){
            let d=Math.hypot(r.lat-branch.lat,r.lng-branch.lng);
            if(d<minDist){minDist=d;nearest={rider:r,index:i};}
        }
    });
    if(!nearest){alert("No available riders!");return;}

    nearest.rider.status='Busy';
    updateRiderStatus();
    moveRider(nearest.rider,riderMarkers[nearest.index],loc);
});

// Update Rider Status
function updateRiderStatus(){
    let ul=document.getElementById('riderStatus');
    ul.innerHTML='';
    riders.forEach(r=>{
        let li=document.createElement('li');
        li.textContent=r.name+" - "+r.status;
        ul.appendChild(li);
    });
}

// Move Rider along predefined road-like path & show ETA
function moveRider(rider,marker,loc){
    let path=[
        [marker.getLatLng().lat,marker.getLatLng().lng],
        [branch.lat,branch.lng],
        [loc.lat,loc.lng]
    ];
    let i=0,step=0.0015;
    if(marker.poly){map.removeLayer(marker.poly);}
    let interval=setInterval(()=>{
        if(i>=path.length-1){
            rider.status='Available';
            updateRiderStatus();
            document.getElementById('etaDisplay').textContent="Delivered!";
            clearInterval(interval);
            return;
        }
        let cur=path[i],next=path[i+1];
        let lat=marker.getLatLng().lat,lng=marker.getLatLng().lng;
        let newLat=lat+(next[0]-cur[0])*step;
        let newLng=lng+(next[1]-cur[1])*step;
        marker.setLatLng([newLat,newLng]);
        if(!marker.poly) marker.poly=L.polyline([[cur[0],cur[1]]],{color:'blue',weight:4}).addTo(map);
        marker.poly.addLatLng([newLat,newLng]);

        // ETA calculation (simple distance/0.0006 degree per sec)
        let remainingDist=Math.hypot(next[0]-newLat,next[1]-newLng);
        let eta=Math.ceil(remainingDist/0.0006); // seconds approximation
        document.getElementById('etaDisplay').textContent=`Estimated Time: ${eta} sec`;

        if(Math.abs(newLat-next[0])<0.0005 && Math.abs(newLng-next[1])<0.0005){i++;}
    },20);
}
