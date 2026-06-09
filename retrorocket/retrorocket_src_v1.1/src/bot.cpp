#include "bot.h"

Bot::Bot(unsigned int id) : LocalPlayer(id)
{
  //   for(unsigned int i = 0; i < retrorocket->ships.size() ; i++)
  //     if( retrorocket->ships[i] == ship-> )
  //       thisid = i;
  //   cout << "thisid: " << thisid << " size of ships" << retrorocket->ships.size() << endl;
  ship = NULL;

  liftoff = true;
  counter = 0;
  checkahead = 30;
  liftoffc = 0;
}
Bot::~Bot()
{
  
}
//void advance()
void Bot::setWeaponA (int weapon)
{
  LocalPlayer::setWeaponA(weapon);
}
void Bot::setWeaponB (int weapon)
{
  LocalPlayer::setWeaponB(weapon);
}
void Bot::shopUpgrade (Upgrades upgrade, bool ignorePrice)
{
  Player::shopUpgrade(upgrade,ignorePrice);
}

bool Bot::checkPossibleCollision(Fix32 nx, Fix32 ny)
{
  if (!dead) {

    // particles
    if (immortal) {
      if (retrorocket->manager->getCollisionDamage (ship)) {
	cout << "colliding with PARTICLE" << endl;
	return true;
	//           playSound(retrorocket->particleSound, retrorocket->particleSoundSize,
	//                     retrorocket->config->getInt("volume_particle_ship"),
	//                     x, y, retrorocket->config->getInt("rate_particle_ship"));
      }
    }

    // sobs
    for (unsigned int s = 0; s < retrorocket->sobs.size(); s++) {
      if (ship->collidesWith (retrorocket->sobs[s])) {
	cout << "colliding with SOB" << endl;
	return true;
	break;
      }
    }

    // Background
    for (int i = 0; i < ship->scp->collisionPoints; i++) {
      Fix32 x_i = ship->scp->points[i].x;
      Fix32 y_i = ship->scp->points[i].y;

      if (retrorocket->field->getCPixel ( nx + x_i, ny + y_i )) {
	cout << "colliding with BG" << endl;
	return true;
      }
    }
//     if (retrorocket->field->getCPixel ( nx , ny  )) {
//       cout << "colliding with BG" << endl;
//       return true;
//     }
    // check ball collision
    for (unsigned int j = 0; j < retrorocket->balls.size(); j++) {
      if (ship->collidesWith (retrorocket->balls[j])) {
	cout << "colliding with ball!?!?!" << endl;
	return true;
      }
    }
  }
  return false;
}

void Bot::advance()
{
  if(id == 1 ) return;
  /*
    do ai action via newActions and:
    ACTION_LEFT, ACTION_LEFT, ACTION_THRUST, ACTION_SHOOT_A and _B
  */
  //
  if(counter == maxsteps)
    exit(0);
  counter++;
  if(!liftoff){
    //     cout << "checking points.. " << endl;
    //     for(int i = 0;i<ship->scp->collisionPoints;i++){
    //       if(ship->scp->points[i].x == 0 && ship->scp->points[i].y == 0)
    // 	cout << " har origo punk i sprite " << endl;
    //       else
    // 	cout << "x: " << ship->scp->points[i].x.toFloat() << " y: " << ship->scp->points[i].y.toFloat() << endl;
    //     }
    //     cout << endl;
    startx = ship->x;
    starty = ship->y;
    
    cout << "doing liftoff" << endl;
    newActions |=  ACTION_THRUST;
    if(liftoffc==10)
      liftoff = true;
    else
      liftoffc ++;
    return LocalPlayer::updateAction();
  }
  else{
    cout << "not liftoff went from " << startx.toFloat() << "," << starty.toFloat()
	 << " to " << ship->x.toFloat() << "," << ship->y.toFloat() << endl; 
  }
  //   cout << "IN ADVANCE!!!!!!!!" << endl;
  oldActions = newActions;
  newActions = 0;

  if (ship) pursueclosest();
  //   exit(0);
  return LocalPlayer::updateAction();
}
void Bot::pursueclosest()
{

  cout << "i pursue" << endl;
  Ship * enemy; Fix32 distance;
  ship->getClosestEnemyShip(&enemy,id,&distance,ship->x,ship->y);
  //  delete enemy;
  cout << "thiship: "<<this->ship<<" enemy: " << enemy << endl;
  if(!enemy || enemy == this->ship){
  }else{//update the coords..
    tox = enemy->x; toy = enemy->y;    
  }
  
  Fix32 Cangle = vectorAngle(tox - ship->x,toy - ship->y);
  Fix32 Bangle = vectorAngle(ship->vx,ship->vy);
  Fix32 alfa = Cangle - Bangle;
  if(alfa.abs() > 90){
    ship->setOrientation(Bangle + Fix32(180));
  }else{
    ship->setOrientation(Bangle + (Fix32(2)*alfa));
  }
  angle = ship->getOrientation();
  //  ship->setOrientation(angle);



  int maxc=0;lastcollision = false;
  int max =  checkFutureCollision(checkahead);//checkFutureCollision(ship->x,ship->y,checkahead);//ship->x, ship->y,checkahead);
  Fix32 maxAngle = 0;
  int curval=RAND_MAX;
  while(curval>0){
    angle = angle+10;
    ship->setOrientation(angle);
    curval = checkFutureCollision(checkahead);
    //    curval = checkFutureCollision(ship->x,ship->y,checkahead);//ship->x, ship->y,checkahead);
    if(curval == 0 || angle > 512){
      maxAngle = angle;
      break;
    }else if(curval>max){
      max = curval;
      maxAngle = angle;
    }
  }
  ship->setOrientation(maxAngle);
  newActions |=  ACTION_THRUST; 
  cout << "maxAngle: " << maxAngle.toInt() << endl;
}
int Bot::checkFutureCollision(int upton)
{
  Fix32 hangle = vectorAngle(ship->vx,ship->vy);
  Fix32 dx= Fix32(2)/Fix32(cos(hangle.toFloat()));
  Fix32 dy= Fix32(2)/Fix32(cos(hangle.toFloat()));
  //Y = hypo(8)/sin(horisvinkel)
  int first = checkFutureCollision(ship->x-dx,ship->y-dy,upton);
  int second = checkFutureCollision(ship->x+dx,ship->y+dy,upton);
  if(first>second)
    return first;
  else
    return second;
}
int Bot::checkFutureCollision(Fix32 fx, Fix32 fy, int upton)
{
  cout << "i checkfuturecoll.. upton:" << upton 
       << " thrust: " << thrust.toFloat() 
       << " vx: " << ship->vx.toFloat() << " vy: " << ship->vy.toFloat() << endl;
  Fix32 tvx; tvx = ship->vx + vectorXComp (ship->getOrientation(), thrust/ship->mass);
  Fix32 tvy; tvy = ship->vy + vectorYComp (ship->getOrientation(), thrust/ship->mass);
  cout << "thrust/ship->mass: " << (thrust/ship->mass).toFloat() 
       << " speed: " << speed() << endl;
  cout << "tvx: " << tvx.toFloat() << " tvy: " << tvy.toFloat() << endl;
  //   Fix32 tx =  ship->x + (tvx * n);
  //   Fix32 ty=  ship->y + (tvy * n);

  
  Fix32 tx,ty;//,ltvx,ltvy; 
  tx = fx; ty = fy;
  for(int i=0;i<upton;i++){
    tvy += vectorYComp (ship->getOrientation(), thrust/ship->mass);
    tvx += vectorXComp (ship->getOrientation(), thrust/ship->mass);
    tx = tx + tvx;
    ty = ty + tvy;

    tvx *= ship->dragCoeff;
    tvy *= ship->dragCoeff;
    tvy += ship->gravityCoeff;
    //     if(ty.toFloat()>=SCREEN_HEIGHT){
    //       cout << "ty bigger than height, ty : " <<  ty.toFloat() << " height "<< SCREEN_HEIGHT<<endl;
    //       return true;
    //     }
    //     if(tx.toFloat()>=SCREEN_WIDTH){
    //       cout << "tx bigger than width, tx : " <<  ty.toFloat() << " width "<< SCREEN_WIDTH<<endl;
    //       return true;
    //     }
    bool hmm = checkPossibleCollision(tx,ty);
    cout << "ltvx: "<<tvx.toFloat()<<" ltvy: "<<tvy.toFloat()
	 <<" tx: " << tx.toFloat() << " ty: " << ty.toFloat() 
	 << " hmm: " << hmm << endl;
//     exit(0);
    if(hmm)
      return i+1;
  }
  return 0;
}
void Bot::registerShip (Ship *ship) {
  Player::registerShip (ship);

  if (weaponAAvailable.size() > 0) {
    ship->weaponA = weaponAAvailable[0];
  } else {
    ship->weaponA = NULL;
  }
  if (weaponBAvailable.size() > 0) {
    ship->weaponB = weaponBAvailable[0];
  } else {
    ship->weaponB = NULL;
  }
}

